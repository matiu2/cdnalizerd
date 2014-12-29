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
#include "inotify.hpp" // Brought from build/3rd_party/src/inotify... by cmake
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
  inotify::Instance inotify;
  std::map<uint32_t, std::string> cookies;
  const Config& config;
  std::vector<WatchGroup> groups;
  Inotify notify;
  void readConfig();
  void watchNewDir(const char* path);
  void onFileSaved(std::string);
  void onDirCreated(std::string);
  void onFileRenamed(std::string, std::string);
  void onFileRemoved(std::string);
public:
  Watcher(Config &config) : config(config.use()) {}
  void watch();
};

}
