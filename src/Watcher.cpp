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
    auto addWatch = [&](const char *path) {
      if (!notify.FindWatch(path)) {
        std::cout << "Watching: " << path << std::endl;
        watches.emplace_back(InotifyWatch(
            path, IN_CREATE | IN_CLOSE_WRITE | IN_DELETE | IN_MOVED_FROM |
                      IN_MOVED_TO | IN_DELETE_SELF));
        notify.Add(watches.back());
      }
    };
    addWatch(entry.local_dir().c_str());
    walkDir(entry.local_dir().c_str(), addWatch);
  }
}

void Watcher::watch() {
  readConfig();
  while (true) {
    try {
      notify.WaitForEvents();
      int count = notify.GetEventCount();
      do {
        InotifyEvent event;
        notify.GetEvent(event);
        std::string name;
        event.GetName(name);
        std::cout << "EVENT" << std::endl;
        if (event.IsType(IN_CLOSE_WRITE))
          std::cout << "Close Write " << name << std::endl;
        else if (event.IsType(IN_CREATE))
          std::cout << "Create " << name << std::endl;
        else if (event.IsType(IN_DELETE))
          std::cout << "Delete " << name << std::endl;
        else if (event.IsType(IN_MOVED_FROM))
          std::cout << "Moved From " << event.GetCookie() << ' ' << name << std::endl;
        else if (event.IsType(IN_MOVED_TO))
          std::cout << "Moved To " << event.GetCookie() << ' ' << name << std::endl;
        else if (event.IsType(IN_DELETE_SELF))
          std::cout << "Delete self " << name << std::endl;
      } while (--count);
    } catch (InotifyException &e) {
      std::cerr << e.GetMessage() << std::endl;
    }
  }
}

}
