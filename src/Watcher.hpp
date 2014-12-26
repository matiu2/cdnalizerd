/// Watches for all file changes, then launches jobs depending on what needs doing
/// This part of the program pulls in all our libraries
/// * inotify - to watch for file changes
/// * config_reader - to set up the rules
/// * curlpp11 - to talk to the API endpoints
/// * jsonpp11 - to understand the API endpoints

// Inotify events to look out for:
// IN_CLOSE_WRITE, IN_DELETE, IN_MOVED_FROM, IN_MOVED_TO

#pragma once

#include <string>

#include "config_reader/config.hpp"
#include "login.hpp"
#include "inotify-cxx.h" // Brought from build/3rd_party/src/inotify... by cmake
#include "utils.hpp"

#include <map>

namespace cdnalizerd {

struct WatchGroup {
  Rackspace& login;
  const std::string& directory; // References the string in the config directly
  std::string url; // Built with 'getContainerUrl'
};

struct Watcher {
private:
  std::map<std::string, Rackspace> logins; // maps username to login
  const Config& config;
  std::vector<WatchGroup> groups;
  Inotify notify;
  void readConfig(const Config& config) {
    for (const auto &entry : config.entries()) {
      Rackspace& login = logins[entry.username()];
      if (!login)
        login.login(entry.username(), entry.apikey());
      InotifyWatch watch(entry.local_dir(), IN_ALL_EVENTS | IN_CLOSE_WRITE |
                                                IN_DELETE | IN_MOVED_FROM |
                                                IN_MOVED_TO);
      notify.Add(watch);
    }
  }
public:
  Watcher(Config &config) : config(config.use()) {}
};

}
