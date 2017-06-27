#pragma once

#include "errors.hpp"

#include <algorithm>
#include <memory>
#include <vector>
#include <string>
#include <sstream>
#ifndef NDEBUG
#include <cassert>
#endif

namespace cdnalizerd {

using strings = std::vector<std::string>;

using sstring = std::shared_ptr<std::string>;

/// Holds a single mapping from a local folder to remote location
struct ConfigEntry {
  sstring username;
  sstring apikey;
  sstring region;
  sstring container;
  bool snet;
  bool move; // Move file to cloud instead of just copy
  sstring local_dir;
  sstring remote_dir;
  ConfigEntry(sstring username, sstring apikey, sstring region,
              sstring container, bool snet, bool move, sstring local_dir,
              sstring remote_dir)
      : username(username), apikey(apikey), region(region),
        container(container), snet(snet), move(move), local_dir(local_dir),
        remote_dir(remote_dir) {}

  ConfigEntry() = default;
  ConfigEntry(const ConfigEntry&) = default;
  ConfigEntry(ConfigEntry&&) = default;

  ConfigEntry &operator=(const ConfigEntry &other) = default;

  // To allow easy sorting
  bool operator<(const ConfigEntry &other) const {
    return (*local_dir < *other.local_dir) ||
           (*remote_dir < *other.remote_dir) || (*username < *other.username) ||
           (*region < *other.region) || (*container < *other.container) ||
           (*apikey < *other.apikey) || (snet < other.snet) ||
           (move < other.move);
  }
  bool operator<(const std::string &path) const { return *local_dir < path; }
  bool operator==(const ConfigEntry &other) const {
    return (*local_dir == *other.local_dir) &&
           (*remote_dir == *other.remote_dir) &&
           (*username == *other.username) && (*region == *other.region) &&
           (*container == *other.container) && (*apikey == *other.apikey) &&
           (snet == other.snet) && (move == other.move);
  }
};

#ifndef NDEBUG
/// For debugging, make sure that we never use the list of Entries unsorted.
using EntriesBase = std::vector<ConfigEntry>;
struct Entries : public EntriesBase {
  bool is_sorted = false;
  void push_back(const value_type &val) {
    is_sorted = false;
    EntriesBase::push_back(val);
  }
  void push_back(value_type &&val) {
    is_sorted = false;
    EntriesBase::push_back(val);
  }
};
#else
using Entries = std::vector<ConfigEntry>;
#endif

/// Holds our configuration
class Config {
private:
  Entries _entries;
  ConfigEntry lastEntry;

public:
  void addUsername(std::string username) { lastEntry.username.reset(new std::string(std::move(username))); }
  void addApikey(std::string apikey) { lastEntry.apikey.reset(new std::string(std::move(apikey))); }
  void addRegion(std::string region) { lastEntry.region.reset(new std::string(std::move(region))); }
  void addContainer(std::string container) { lastEntry.container.reset(new std::string(std::move(container))); }
  /// Toggles service net on and off
  void setSNet(bool new_val) {
    lastEntry.snet = new_val;
  }
  /// Toggles move to cloud (instead of just copy) on and off
  void setMove(bool new_val) {
    lastEntry.move = new_val;
  } 
  void addEntry(std::string local_dir, std::string remote_dir) {
    ConfigEntry result(lastEntry);
    result.local_dir.reset(new std::string(std::move(local_dir)));
    result.remote_dir.reset(new std::string(std::move(remote_dir)));
    _entries.emplace_back(std::move(result));
  }

  Config() = default;

  Config(const Config &other)
      : _entries(other._entries), lastEntry(other.lastEntry) {}
  Config(Config &&other)
      : _entries(std::move(other._entries)), lastEntry(other.lastEntry) {}

  Config &operator=(const Config &other) {
    _entries = other._entries;
    lastEntry = other.lastEntry;
    return *this;
  }

  /// Makes the config ready for use
  const Config &use() {
    std::sort(_entries.begin(), _entries.end());
#ifndef NDEBUG
    _entries.is_sorted = true;
#endif
    return *this;
  }

  ConfigEntry getEntryByPath(const std::string &path) const {
#ifndef NDEBUG
    assert(_entries.is_sorted);
#endif
    auto found = std::lower_bound(_entries.cbegin(), _entries.cend(), path);
    if (found != _entries.cend())
      return *found;
    else {
      std::stringstream msg;
      msg << "Couldn't find " << path << " in this list: ";
      for (auto &e : _entries)
        msg << *e.local_dir << ", ";
      throw std::logic_error(msg.str());
    }
  }
  const Entries &entries() const { return _entries; }
  /// Returns true if we have a config
  operator bool() const { return _entries.size() > 0; }
};
}
