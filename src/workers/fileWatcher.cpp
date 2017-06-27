#include "fileWatcher.hpp"

#include "../inotify.hpp"

#include "queueWorker.hpp"

namespace cdnalizerd {
namespace workers {

constexpr int maskToFollow =
    IN_CREATE | IN_CLOSE_WRITE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO;

void readConfig(yield_context &yield, Status &status, const Config &config,
                inotify::Instance &inotify) {
  for (const ConfigEntry &entry : config.entries()) {
    // Starts watching a directory
    inotify::Watch &watch =
        inotify.addWatch(entry.local_dir->c_str(), maskToFollow);
    status.watchToConfig[watch.handle()] = entry;
    walkDir(entry.local_dir->c_str(), [&](const char *path) {
      if (!inotify.alreadyWatching(path))
        inotify.addWatch(path, maskToFollow);
    });
  }
}

void handleEvent(Status &status, inotify::Event &&event) {
  const ConfigEntry &entry = status.watchToConfig[event.watch().handle()];
  // This may be a move or uplod operation
  Operation uploadOperation = entry.move ? Move : Upload;
  // We'll use this in several places below
  std::string serverSource =
      joinPaths(joinPaths(*entry.container, *entry.remote_dir),
                unJoinPaths(*entry.local_dir, event.path()));

  if (event.wasSaved() && !event.isDir()) {
    // Simple upload / move
    status.jobsToDo[entry].emplace_back(
        Job{uploadOperation, event.path(),
            joinPaths(*entry.container,
                      unJoinPaths(*entry.local_dir, event.path()))});
  } else if (event.wasMovedFrom()) {
    // This may be a server side rename (copy + delete)
    if (event.destination) {
      const ConfigEntry &dest =
          status.watchToConfig[event.destination->watch().handle()];
      std::string serverDestinaton =
          joinPaths(joinPaths(*dest.container, *dest.remote_dir),
                    unJoinPaths(*dest.local_dir, event.destination->path()));

      if ((*dest.username == *entry.username) &&
          ((*dest.region == *entry.region))) {
        // If the source and the destination are in the same region and use the
        // same login, server side copy, then delete.
        status.jobsToDo[entry].emplace_back(
            Job{Copy, serverSource, serverDestinaton,
                std::unique_ptr<Job::Second>(
                    new Job::Second{entry, Job{Delete, serverSource}})});
      } else {
        // The file is being moved from one account to another or from one region to another
        // We'll just upload to one account, then delete on the other
        const ConfigEntry &dest =
            status.watchToConfig[event.destination->watch().handle()];
        status.jobsToDo[dest].emplace_back(
            Job{uploadOperation, event.path(), serverDestinaton,
                std::unique_ptr<Job::Second>(
                    new Job::Second{dest, Job{Delete, serverSource}})});
      }
    } else {
      // This file was move out of our known directory space, delete it from the server
      status.jobsToDo[entry].emplace_back(Job{Delete, serverSource});
    }
  } else if (event.wasMovedTo()) {
    // This file was moved from some unknown source to our space. Upload it to
    // the server
    status.jobsToDo[entry].emplace_back(
        Job{uploadOperation, event.path(), serverSource});
  } else if (event.wasDeleted()) {
    status.jobsToDo[entry].emplace_back(
        Job{Delete, serverSource});
  }
}

void watchForFileChanges(yield_context &yield, Status &status,
                         const Config &config) {
  // Setup
  inotify::Instance inotify(yield);
  readConfig(yield, status, config, inotify);

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
      // ... launch the appropriate server worker
      handleEvent(status, std::move(event));
    }
  }
  // Now spawn up to 10 workers for all the jobs in the queue
  int spawnCount = std::min(status.jobsToDo.size(), size_t(10));
  for (int i = 0; i < spawnCount; ++i) {
    RESTClient::http::spawn([&](yield_context y) { queueWorker(y, status, config); });
  }
}
    

} /* workers  */ 
} /* cdnalizerd  */ 
