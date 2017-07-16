#include "mainProcess.hpp"

#include "../inotify.hpp"
#include "../logging.hpp"

#include "uploader.hpp"
#include "../AccountCache.hpp"
#include "../WorkerManager.hpp"

#include "../globals.hpp"

#include <boost/log/trivial.hpp>

namespace cdnalizerd {
namespace processes {

constexpr int maskToFollow =
    IN_CREATE | IN_CLOSE_WRITE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO;

// Maps an inotify cookie to the event that last had that cookie
using Cookies = std::map<uint32_t, inotify::Event>;

// Maps inotify watch handles to config entries
using WatchToConfig = std::map<uint32_t, ConfigEntry>; 

/// Reads our configuration object and creates all the inotify watches needed
void createINotifyWatches(inotify::Instance &inotify, WatchToConfig& watchToConfig) {
  for (const ConfigEntry &entry : config.entries()) {
    // Starts watching a directory
    inotify::Watch &watch =
        inotify.addWatch(entry.local_dir.c_str(), maskToFollow);
    watchToConfig[watch.handle()] = entry;
    walkDir(entry.local_dir.c_str(), [&](const char *path) {
      if (!inotify.alreadyWatching(path))
        inotify.addWatch(path, maskToFollow);
    });
  }
}

void watchForFileChanges(yield_context yield) {
  BOOST_LOG_TRIVIAL(info) << "Creating inotify watches...";
  // Setup
  inotify::Instance inotify(yield);
  // Maps inotify watch handles to config entries
  std::map<uint32_t, ConfigEntry> watchToConfig;
  createINotifyWatches(inotify, watchToConfig);

  // Account login information
  AccountCache accounts;

  // Spawn some workers to fill in the account info (token and urls)
  BOOST_LOG_TRIVIAL(info) << "Getting API Authentication tokens...";
  boost::asio::deadline_timer waitForLogins(*RESTClient::tcpip::getService(),
                                            boost::posix_time::minutes(10));
  int loginWorkers = 2;
  for (int i = 0; i != loginWorkers; ++i) {
    RESTClient::http::spawn([&](yield_context y) {
      fillAccountCache(y, config, accounts, [&loginWorkers, &waitForLogins]() {
        --loginWorkers;
        if (loginWorkers == 0)
          waitForLogins.cancel();
      });
    });
  }

  // Wait for all the logins to finish
  waitForLogins.async_wait(yield);

  // Logins to Rackspace timed out
  assert(loginWorkers == 0);

  std::list<Job> jobsToDo;

  /// Holds file move operations that are waiting for a pair
  std::map<uint32_t, inotify::Event> cookies;

  WorkerManager workers;

  while (true) {
    inotify::Event event = inotify.waitForEvent();
    // If it's a move event, find its pair
    if (event.cookie) {
      auto found = cookies.find(event.cookie);
      if (found != cookies.end()) {
        if (event.wasMovedTo())
          std::swap(event, found->second);
        if (event.wasMovedFrom())
          event.destination.reset(new inotify::Event(std::move(found->second)));
        else {
          /// TODO: Log a runtime error here, only move events should have
          /// cookies
          cookies.erase(found);
        }
      } else {
        // The event's partner hasn't been found yet.
        // Store it in cookies; start another worker that'll wait 1 second, then
        // launch it as a delete if it's still there
        // TODO: Launch the cleanup worker
        continue; // Wait for its partner to appear
      }
    }
    // Now we have a whole event...
    if (event.wasCreated()) {
      // ... if a directory was created, add a watch to it
      std::string path = event.path();
      if (isDir(path.c_str()))
        inotify.addWatch(path.c_str(), maskToFollow);
    } else {
      // Create the job
      const ConfigEntry &entry = watchToConfig[event.watch().handle()];
      Rackspace &rs = accounts[entry.username];
      Job job{entry.move ? Move : Upload,
              joinPaths(entry.local_dir, event.path(),
                        joinPaths(rs.getURL(*entry.region), *entry.container,
                                  entry.remote_dir,
                                  unJoinPaths(entry.local_dir, event.path())))};
      // If this event is a move and has a destionation..
      if (event.wasMovedFrom()) {
        if (event.destination) {
          // This may be a server side rename (copy + delete)
          const ConfigEntry &dest =
              watchToConfig[event.destination->watch().handle()];
          // The original job should be a server side copy
          job.operation = SCopy;
          job.dest = joinPaths(rs.getURL(*dest.region), *dest.container,
                                dest.remote_dir,
                                unJoinPaths(dest.local_dir, event.path()));
          // The follow up job is the server side delete
          job.next.reset(new Job{SDelete, job.source});
        } else {
          // This file was moved out of our known directory space, delete it
          // from the server
          job.operation = SDelete;
          job.dest.clear();
        }
        // Put it in the right queue
        auto worker = workers.getWorker(job.workerURL(), rs);
        worker->addJob(std::move(job));
      } else if (event.wasDeleted()) {
        job.operation = SDelete;
        job.source.swap(job.dest);
        job.dest.clear();
      }
    }
  }
}

} /* processes  */ 
} /* cdnalizerd  */ 
