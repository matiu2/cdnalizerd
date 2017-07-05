#include "fileWatcher.hpp"

#include "../inotify.hpp"

#include "queueWorker.hpp"
#include "../AccountCache.hpp"

namespace cdnalizerd {
namespace workers {

constexpr int maskToFollow =
    IN_CREATE | IN_CLOSE_WRITE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO;

void readConfig(yield_context &yield, Status &status, const Config &config,
                inotify::Instance &inotify) {
  for (const ConfigEntry &entry : config.entries()) {
    // Starts watching a directory
    inotify::Watch &watch =
        inotify.addWatch(entry.local_dir.c_str(), maskToFollow);
    status.watchToConfig[watch.handle()] = entry;
    walkDir(entry.local_dir.c_str(), [&](const char *path) {
      if (!inotify.alreadyWatching(path))
        inotify.addWatch(path, maskToFollow);
    });
  }
}

Job handleEvent(Status &status, inotify::Event &&event) {
  const ConfigEntry &entry = status.watchToConfig[event.watch().handle()];
  // This may be a move or uplod operation
  Operation uploadOperation = entry.move ? Move : Upload;
  // We'll use this in several places below
  std::string serverSource =
      joinPaths(joinPaths(*entry.container, entry.remote_dir),
                unJoinPaths(entry.local_dir, event.path()));

  if (event.wasSaved() && !event.isDir()) {
    // Simple upload / move
    return Job{entry, uploadOperation, event.path(),
               joinPaths(*entry.container,
                         unJoinPaths(entry.local_dir, event.path()))};
  } else if (event.wasMovedFrom()) {
    // This may be a server side rename (copy + delete)
    if (event.destination) {
      const ConfigEntry &dest =
          status.watchToConfig[event.destination->watch().handle()];
      std::string serverDestinaton =
          joinPaths(joinPaths(*dest.container, dest.remote_dir),
                    unJoinPaths(dest.local_dir, event.destination->path()));

      if ((*dest.username == *entry.username) &&
          ((*dest.region == *entry.region))) {
        // If the source and the destination are in the same region and use the
        // same login, server side copy, then delete.
        return Job {
          entry, Copy, serverSource, serverDestinaton,
              std::shared_ptr<Job>(
                  new Job{Job{entry, Delete, serverSource}})
        };
      } else {
        // The file is being moved from one account to another or from one
        // region to another
        // We'll just upload to one account, then delete on the other
        const ConfigEntry &dest =
            status.watchToConfig[event.destination->watch().handle()];
        return Job{entry, uploadOperation, event.path(), serverDestinaton,
                   std::shared_ptr<Job>(
                       new Job{Job{dest, Delete, serverSource}})};
      }
    } else {
      // This file was moved out of our known directory space, delete it from
      // the server
      return Job{entry, Delete, serverSource};
    }
  } else if (event.wasMovedTo()) {
    // This file was moved from some unknown source to our space. Upload it to
    // the server
    return Job{entry, uploadOperation, event.path(), serverSource};
  } else if (event.wasDeleted()) {
    return Job{entry, Delete, serverSource};
  }
  return Job{};
}

void queueJob(Job &&job, Status &status, const Config &config, AccountCache& accounts) {
  // Find or create the worker for this job
  std::list<Worker> workers =
      status.workers[{job.configuration.username, job.configuration.region}];
  // We like to have up to 3 workers per connection
  if (workers.size() < 3) {
    // Create a new worker
    Rackspace& rs = accounts[job.configuration.username];
    Worker worker(workers, rs);
    worker.jobsToDo.emplace_back(std::move(job));
    assert(rs.status() == Rackspace::Ready);
    worker.rs = rs;
    workers.emplace_back(std::move(worker));
    workers.back().me = --workers.end();
    RESTClient::http::spawn(
        [&](yield_context y) { queueWorker(y, status, workers.back()); });
  } else {
    // Find which worker has the least jobs
    auto worker = std::min_element(
        workers.begin(), workers.end(), [](const Worker &a, const Worker &b) {
          return a.jobsToDo.size() < b.jobsToDo.size();
        });
    worker->jobsToDo.emplace_back(std::move(job));
  }
}

void watchForFileChanges(yield_context &yield, Status &status,
                         const Config &config) {
  // Setup
  inotify::Instance inotify(yield);
  readConfig(yield, status, config, inotify);
  AccountCache accounts;

  // Spawn some workers to fill in the account info (token and urls)
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

  assert("Logins to Rackspace timed out");

  std::list<Job> jobsToDo;

  while (true) {
    inotify::Event event = inotify.waitForEvent();
    // If it's a move event, find its pair
    if (event.cookie) {
      auto found = status.cookies.find(event.cookie);
      if (found != status.cookies.end()) {
        if (event.wasMovedTo())
          std::swap(event, found->second);
        if (event.wasMovedFrom())
          event.destination.reset(new inotify::Event(std::move(found->second)));
        /// TODO: Log a runtime error here, only move events should have cookies
        status.cookies.erase(found);
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
      // Get the next job that needs doing
      Job job{handleEvent(status, std::move(event))};
      // Put it in the right queue
      queueJob(std::move(job), status, config, accounts);
    }
  }
}
    

} /* workers  */ 
} /* cdnalizerd  */ 
