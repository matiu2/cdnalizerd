#include "Watcher.hpp"

#include <vector>

#include <iostream>

namespace cdnalizerd {

void Watcher::readConfig() {
  for (const auto &entry : config.entries()) {
    std::unique_ptr<Rackspace> &login = logins[entry.username()];
    if (!login)
      login.reset(new Rackspace(yield));
    login->login(entry.username(), entry.apikey());
    // Starts watching a directory
    watchNewDir(entry.local_dir().c_str());
    walkDir(entry.local_dir().c_str(),
            std::bind(&Watcher::watchNewDir, this, std::placeholders::_1));
  }
}

void Watcher::watchNewDir(const char *path) {
  if (!inotify.alreadyWatching(path)) {
    inotify.addWatch(path, IN_CREATE | IN_CLOSE_WRITE | IN_DELETE |
                               IN_MOVED_FROM | IN_MOVED_TO | IN_DELETE_SELF);
  }
}

void Watcher::watch() {
  readConfig();
  uint32_t lastCookie = 0; // When a file is moved, it's given a cookie, if we
                           // get a from=>to with the same cookie, it's a rename
  bool lastEventWasMoveFrom =
      false; // True if the last event was move-from - helps us t
  while (true) {
    try {
      // Any 'move-from' events that didn't get a 'move-to' become delete events
      if (cookies.size() > 0)
        for (auto &cookie : cookies)
          onFileRemoved(cookie.second);
      cookies.clear();
      for (const auto &event : inotify.waitForEvents()) {
        auto path = event.path();
        if (event.wasSaved()) {
          // A file has just been saved
          onFileSaved(path);
        } else if (event.wasCreated() && event.isDir()) {
          // A new directory was created
          watchNewDir(path.c_str());
        } else if (event.wasDeleted()) {
          // A file or directory was deleted
          // TODO: If the file is a link or a symlink, don't trigger an event because we don't trigger 
          // TODO: later maybe follow symlinks.
          onFileRemoved(path);
        } else if (event.wasMovedFrom()) {
          // A file or dir was moved somewhere
          // The move operation generates a cookie that we can use later to tell
          // if it landed back in our tree
          cookies.insert(std::make_pair(event.cookie, path));
        } else if (event.wasMovedTo()) {
          // A file or dir move has completed
          auto found = cookies.find(event.cookie);
          if (found != cookies.end()) {
            // The move operation started in our tree, so this is a rename for
            // cloud files
            onFileRenamed(found->second, path);
            cookies.erase(found);
          } else {
            // The move operation originated else where so this is a create for
            // cloud files
            onFileSaved(path);
          }
        }
      }
    } catch (std::logic_error &e) {
      std::cerr << e.what() << std::endl;
    } catch (std::system_error &e) {
      std::cerr << e.what() << std::endl;
    }
  }
}

void Watcher::onFileSaved(std::string name) {
  std::cout << "File Saved: " << name << std::endl;
}

void Watcher::onDirCreated(std::string name) {
  std::cout << "Dir created: " << name << std::endl;
}

void Watcher::onFileRenamed(std::string from, std::string to) {
  std::cout << "File Renamed: " << from << " to " << to << std::endl;
}

void Watcher::onFileRemoved(std::string name) {
  std::cout << "File Removed: " << name << std::endl;
}
}
