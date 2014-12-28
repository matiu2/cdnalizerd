#include "Watcher.hpp"

#include <vector>

#include <iostream>

namespace cdnalizerd {

void Watcher::readConfig() {
  for (const auto &entry : config.entries()) {
    Rackspace &login = logins[entry.username()];
    if (!login)
      login.login(entry.username(), entry.apikey());
    // Starts watching a directory
    watchNewDir(entry.local_dir().c_str());
    walkDir(entry.local_dir().c_str(), std::bind(&Watcher::watchNewDir, this, std::placeholders::_1));
  }
}

void Watcher::watchNewDir(const char *path) {
  if (!notify.FindWatch(path)) {
    std::cout << "Watching: " << path << std::endl;
    watches.emplace_back(InotifyWatch(path, IN_CREATE | IN_CLOSE_WRITE |
                                                IN_DELETE | IN_MOVED_FROM |
                                                IN_MOVED_TO | IN_DELETE_SELF));
    notify.Add(watches.back());
  }
}

void Watcher::watch() {
  readConfig();
  uint32_t lastCookie = 0; // When a file is moved, it's given a cookie, if we get a from=>to with the same cookie, it's a rename
  bool lastEventWasMoveFrom = false; // True if the last event was move-from - helps us t
  while (true) {
    try {
      // Any 'move-from' events that didn't get a 'move-to' become delete events
      if (cookies.size() > 0)
        for (auto& cookie : cookies)
          onFileRemoved(cookie.second);
      cookies.clear();
      notify.WaitForEvents();
      int count = notify.GetEventCount();
      do {
        InotifyEvent event;
        notify.GetEvent(event);
        std::string name;
        event.GetName(name);
        auto path = event.GetWatch()->GetPath();
        joinPaths(path, name);
        std::cout << "EVENT" << std::endl;
        if (event.IsType(IN_CLOSE_WRITE)) {
          // A file has just been saved
          onFileSaved(path);
        } else if (event.IsType(IN_CREATE) && event.IsType(IN_ISDIR)) {
          // A new directory was created
          watchNewDir(path.c_str());
        } else if (event.IsType(IN_DELETE)) {
          // A file or directory was deleted
          // TODO: If it was a dir, stop watching it and all its sub dirs
          onFileRemoved(path);
        } else if (event.IsType(IN_MOVED_FROM)) {
          // A file or dir was moved somewhere
          // The move operation generates a cookie that we can use later to tell if it landed back in our tree
          cookies.insert(std::make_pair(event.GetCookie(), path));
          std::cout << "Moved From " << event.GetCookie() << ' ' << path << std::endl;
        } else if (event.IsType(IN_MOVED_TO)) {
          // A file or dir move has completed
          auto found = cookies.find(event.GetCookie());
          if (found != cookies.end()) {
            // The move operation started in our tree, so this is a rename for cloud files
            onFileRenamed(found->second, path);
            cookies.erase(found);
          } else {
            // The move operation originated else where so this is a create for cloud files
            // TODO: If it is a dir, recursively follow it
            onFileSaved(path);
          }
        } else if (event.IsType(IN_DELETE_SELF)) {
          std::cout << "Delete self " << path << std::endl;
        }
      } while (--count);
    } catch (InotifyException &e) {
      std::cerr << e.GetMessage() << std::endl;
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
