#include "mainProcess.hpp"

#include "../inotify.hpp"
#include "login.hpp"
#include "syncAllDirectories.hpp"
#include "../WorkerManager.hpp"
#include "../jobs/upload.hpp"

#include <boost/log/trivial.hpp>
#include <boost/filesystem.hpp>

#include <RESTClient/http/url.hpp>

namespace cdnalizerd {
namespace processes {

namespace fs = boost::filesystem;
using RESTClient::http::URL;

constexpr int maskToFollow =
    IN_CREATE | IN_CLOSE_WRITE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO;

// Maps an inotify cookie to the event that last had that cookie
using Cookies = std::map<uint32_t, inotify::Event>;

// Maps inotify watch handles to config entries
using WatchToConfig = std::map<uint32_t, ConfigEntry>; 

/// Reads our configuration object and creates all the inotify watches needed
void createINotifyWatches(inotify::Instance &inotify, WatchToConfig& watchToConfig, const Config& config) {
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

void watchForFileChanges(yield_context yield, const Config& config) {
  std::clog << "INFO: Creating inotify watches..." << std::endl;
  // Setup
  inotify::Instance inotify(yield);
  // Maps inotify watch handles to config entries
  std::map<uint32_t, ConfigEntry> watchToConfig;
  createINotifyWatches(inotify, watchToConfig, config);

  // Account login information
  AccountCache accounts;
  login(yield, accounts, config);

  WorkerManager workers;

  syncAllDirectories(yield, accounts, config, workers);

  /// Holds file move operations that are waiting for a pair
  std::map<uint32_t, inotify::Event> cookies;


  std::clog << "DEBUG: Waiting for file events" << std::endl;
  while (true) {
    inotify::Event event = inotify.waitForEvent();
    std::clog << "DEBUG: Got an inotify event: " << event << std::endl;

    // Get the job data ready
    const ConfigEntry &entry = watchToConfig[event.watch().handle()];
    Rackspace &rs = accounts[entry.username];
    fs::path localFile(event.path());
    URL url(rs.getURL(*entry.region, entry.snet));
    auto worker = workers.getWorker(url, rs);
    std::string localRelativePath(
        fs::relative(event.path(), entry.local_dir).string());

    // If the file was closed and may have been written, upload if checksum is different
    if (event.wasClosed()) {
      std::clog << "DEBUG: File was closed for writing: " << localFile.native()
                << std::endl;
      worker->addJob(jobs::makeConditionalUploadJob(
          localFile,
          url / *entry.container / entry.remote_dir / localRelativePath));
    }

    /*
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
      fs::path path = event.path();
      if (fs::is_directory(path)) {
        inotify.addWatch(path.c_str(), maskToFollow);
        continue;
      }
      // TODO: Sometimes files are created with > zero bytes. Check the file
      // size; if it's > 0, upload it
    }
    if (event.wasClosed()) {
      // File was closed; could have been written.
      // Upload if newer than the remote

    }

    // If this event is a move and has a destionation..
    if (event.wasMovedFrom()) {
      if (event.destination) {
        // This may be a server side rename (copy + delete)
        const ConfigEntry &dest =
            watchToConfig[event.destination->watch().handle()];
        // The original job should be a server side copy
        worker->addJob(makeServerSideMoveJob(
            remoteURL, rs.getURL(*dest.region, dest.snet) / *dest.container /
                           dest.remote_dir /
                           unJoinPaths(dest.local_dir, event.path())));
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
      */
  }
}

} /* processes  */ 
} /* cdnalizerd  */ 
