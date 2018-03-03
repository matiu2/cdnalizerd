#pragma once

#include "errors.hpp"
#include "../common.hpp"

#include <algorithm>
#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <regex>
#ifndef NDEBUG
#include <cassert>
#endif

namespace cdnalizerd {

/// Holds a single mapping from a local folder to remote location
struct ConfigEntry {
  std::string username;
  std::string apikey;
  std::string region;
  std::string container;
  bool snet;
  bool move; // Move file to cloud instead of just copy
  std::string local_dir;
  std::string remote_dir;
  std::vector<std::regex> filesToIgnore;
  std::vector<std::regex> directoriesToIgnore;
  ConfigEntry(std::string username, std::string apikey, std::string region,
              std::string container, bool snet, bool move,
              std::string local_dir, std::string remote_dir,
              std::vector<std::regex> filesToIgnore,
              std::vector<std::regex> directoriesToIgnore)
      : username(username), apikey(apikey), region(region),
        container(container), snet(snet), move(move),
        local_dir(std::move(local_dir)), remote_dir(std::move(remote_dir)),
        filesToIgnore(std::move(filesToIgnore)),
        directoriesToIgnore(std::move(directoriesToIgnore)) {}

  ConfigEntry() = default;
  ConfigEntry(const ConfigEntry&) = default;
  ConfigEntry(ConfigEntry&&) = default;

  ConfigEntry &operator=(const ConfigEntry &other) = default;

  // Some functionality methods
  /// Returns true if a file should be ignored
  bool shouldIgnoreFile(const std::string &fileName) const;
  /// Returns true if a directory should be ignored
  bool shouldIgnoreDirectory(const std::string &dirName) const;

  /// Add a new file to ignore
  void addFileToIgnore(std::regex file) {
    filesToIgnore.emplace_back(std::move(file));
  }
  /// Add a new directory to ignore
  void addDirectoryToIgnore(std::regex directory) {
    directoriesToIgnore.emplace_back(std::move(directory));
  }

  /// Returns true if the config entry is ready for use
  bool validate() const {
    return (!username.empty()) && (!apikey.empty()) && (!region.empty()) &&
           (!container.empty()) && (!local_dir.empty()) &&
           (!remote_dir.empty());
  }

  // To allow easy sorting
  bool operator<(const ConfigEntry &other) const {
    return (local_dir < other.local_dir) ||
           (remote_dir < other.remote_dir) || (username < other.username) ||
           (region < other.region) || (container < other.container) ||
           (apikey < other.apikey) || (snet < other.snet) ||
           (move < other.move);
  }
  bool operator==(const std::string &path) const { return local_dir == path; }
  bool operator==(const ConfigEntry &other) const {
    return (local_dir == other.local_dir) && (remote_dir == other.remote_dir) &&
           (username == other.username) && (region == other.region) &&
           (container == other.container) && (apikey == other.apikey) &&
           (snet == other.snet) && (move == other.move);
  }
};

inline bool operator<(const ConfigEntry &entry, const std::string &path) {
  return entry.local_dir < path;
}

inline bool operator<(const std::string &path, const ConfigEntry &entry) {
  return path < entry.local_dir;
}

inline bool operator==(const std::string &path, const ConfigEntry &entry) {
  return path == entry.local_dir;
}

  

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

public:
  Config() = default;

  Config(const Config &other) : _entries(other._entries) {}
  Config(Config &&other) : _entries(std::move(other._entries)) {}

  Config &operator=(const Config &other) {
    _entries = other._entries;
    return *this;
  }

  void addEntry(ConfigEntry entry);

  const ConfigEntry& getEntryByPath(const std::string &path) const;
  const Entries &entries() const { return _entries; }
  /// Returns true if we have a config
  operator bool() const { return _entries.size() > 0; }
};
}
