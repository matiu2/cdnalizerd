#pragma once

#include <map>
#include <queue>
#include <list>
#include <memory>

#include <jsonpp11/json_class.hpp>

#include "Rackspace.hpp"
#include "inotify.hpp"
#include "config_reader/config.hpp"

namespace cdnalizerd {

enum Operation {
  Upload, // Upload to server
  Copy,   // Server side copy
  Delete, // Server side Delete
  Move    // Upload, then local remove
};

struct Job {
  Operation operation;
  std::string sourcePath;
  std::string destPath = "";
  // If set, it should be followed by the next job immediately. 
  // eg. server side copy followed by server side delete.
  // eg. upload to one account, then delete from the other account
  using Second = std::pair<ConfigEntry, Job>;
  std::unique_ptr<Second> next = {};
};


/// Status of the whole app; one instance will be shared by all the workers
struct Status {
  std::map<std::string, std::pair<std::string, json::JSON>>
      logins; // maps username to {token, login_json} pair
  std::map<uint32_t, ConfigEntry> watchToConfig; // Maps inotify watch handles to config entries
  /// Holds file move operations that are waiting for a pair
  std::map<uint32_t, inotify::Event> cookies;
  std::map<ConfigEntry, std::list<Job>> jobsToDo;
  std::map<ConfigEntry, std::list<Job>> jobsInProgress;
};

} /* cdnalizerd  */ 
