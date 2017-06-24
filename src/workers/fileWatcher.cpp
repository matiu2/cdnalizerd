#include "fileWatcher.hpp"

#include "../inotify.hpp"
#include "../handleEvent.hpp"

namespace cdnalizerd {
namespace workers {

constexpr int maskToFollow =
    IN_CREATE | IN_CLOSE_WRITE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO;

void readConfig(yield_context &yield, Status &status, const Config &config,
                inotify::Instance &inotify) {
  for (const auto &entry : config.entries()) {
    std::unique_ptr<Rackspace> &login = status.logins[entry.username()];
    if (!login)
      login.reset(new Rackspace(yield));
    login->login(entry.username(), entry.apikey());
    // Starts watching a directory
    inotify.addWatch(entry.local_dir().c_str(), maskToFollow);
    walkDir(entry.local_dir().c_str(), [&](const char *path) {
      if (!inotify.alreadyWatching(path))
        inotify.addWatch(path, maskToFollow);
    });
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
      handleEvent(std::move(event));
    }
  }
}
    

} /* workers  */ 
} /* cdnalizerd  */ 
