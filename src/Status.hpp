#pragma once

#include <map>
#include <queue>
#include <list>
#include <memory>

#include <jsonpp11/json_class.hpp>

#include "AccountCache.hpp"
#include "Rackspace.hpp"
#include "inotify.hpp"
#include "config_reader/config.hpp"

namespace cdnalizerd {

using namespace std::string_literals;

enum Operation {
  Upload, // Upload to server
  Copy,   // Server side copy
  Delete, // Server side Delete
  Move    // Upload, then local remove
};

struct Job {
  ConfigEntry configuration;
  Operation operation;
  std::string sourcePath;
  std::string destPath = "";
  // If set, it should be followed by the next job immediately. 
  // eg. server side copy followed by server side delete.
  // eg. upload to one account, then delete from the other account
  std::shared_ptr<Job> next = {};
};

struct Worker {
  // A worker will tend to stick to its same URL so it can reuse the connection
  std::list<Job> jobsToDo;
  std::list<Job> jobsInProgress;
  // Mechanism to stop working when we have no new jobs
  std::list<Worker>& list;
  std::list<Worker>::const_iterator me;
  // Info for making connections
  Rackspace& rs;
  void die() { list.erase(me); }
  // Constructor
  Worker(std::list<Worker>& list, Rackspace& rs) : list(list), rs(rs) {}
};

// The two things that a connection type needs to be unique
struct ConnectionInfo {
  sstring username;
  sstring region;
  bool operator<(const ConnectionInfo& other) const {
    std::string empty;
    const std::string& username1(username ? *username : empty);
    const std::string& region1(region ? *region : empty);
    const std::string& username2(username ? *other.username : empty);
    const std::string& region2(region ? *other.region : empty);
    if (username1 < username2)
      return true;
    if (username1 == username2)
      return region1 < region2;
    return false;
  }
};

/// Status of the whole app; one instance will be shared by all the workers
struct Status {
  std::map<std::string, std::pair<std::string, json::JSON>>
      logins; // maps username to {token, login_json} pair
  std::map<uint32_t, ConfigEntry> watchToConfig; // Maps inotify watch handles to config entries
  /// Holds file move operations that are waiting for a pair
  std::map<uint32_t, inotify::Event> cookies;
  std::map<ConnectionInfo, std::list<Worker>> workers;
};

} /* cdnalizerd  */ 
