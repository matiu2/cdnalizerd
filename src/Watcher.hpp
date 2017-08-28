/// TODO: Delete this file, as it is moving into processes/mainProcess.hpp
/// Watches for all file changes, then launches jobs depending on what needs
/// doing. This part of the program pulls in all our libraries
/// * inotify - to watch for file changes
/// * config_reader - to set up the rules

// Inotify events to look out for:
// IN_CLOSE_WRITE, IN_DELETE, IN_MOVED_FROM, IN_MOVED_TO

#pragma once

#include <string>
#include <map>

#include "config_reader/config.hpp"
#include "login.hpp"
#include "inotify.hpp"
#include "utils.hpp"

namespace cdnalizerd {

struct Watcher {
private:
  std::map<std::string, std::unique_ptr<Rackspace>> logins; // maps username to login
  yield_context& yield;
  inotify::Instance inotify;
  std::map<uint32_t, std::string> cookies;
  const Config &config;
  void readConfig();
  void watchNewDir(const char *path);
  void onFileSaved(std::string);
  void onDirCreated(std::string);
  void onFileRenamed(std::string, std::string);
  void onFileRemoved(std::string);

public:
  Watcher(yield_context& yield, Config &config) : yield(yield), config(config.use()) {}
  void watch();
};
}
