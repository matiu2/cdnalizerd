#pragma once

#include <map>
#include <unordered_map>
#include <queue>
#include <list>
#include <memory>

#include <boost/functional/hash.hpp>


#include <jsonpp11/json_class.hpp>

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
  bool running = false;
  std::list<Job> jobsToDo;
  std::list<Job> jobsInProgress;
  // Mechanism to stop working when we have no new jobs
  std::list<Worker>& list;
  std::list<Worker>::const_iterator me;
  void die() {
    list.erase(me);
  }
  // Constructor
  Worker(std::list<Worker>& list) : list(list) {}
};


struct ConnectionMarker {
  sstring username;
  sstring region;

  // Hashing for std::unordered_map
  struct Hash {
    size_t operator()(const ConnectionMarker& in) const {
      size_t result = 0;
      boost::hash_combine(result, boost::hash_value(in.username ? *in.username : ""));
      boost::hash_combine(result, boost::hash_value(in.region ? *in.region : ""));
      return result;
    }
  };
  bool operator==(const ConnectionMarker &other) const {
    return ((username ? *username : ""s) ==
            (other.username ? *other.username : ""s)) &&
           ((region ? *region : ""s) == (other.region ? *other.region : ""s));
  }
};

/// Status of the whole app; one instance will be shared by all the workers
struct Status {
  std::unordered_map<std::string, std::pair<std::string, json::JSON>>
      logins; // maps username to {token, login_json} pair
  std::map<uint32_t, ConfigEntry> watchToConfig; // Maps inotify watch handles to config entries
  /// Holds file move operations that are waiting for a pair
  std::map<uint32_t, inotify::Event> cookies;
  std::unordered_map<ConnectionMarker, std::list<Worker>, ConnectionMarker::Hash> workers;
};

} /* cdnalizerd  */ 
