#pragma once

#include "errors.hpp"

#include <algorithm>
#include <vector>
#include <string>
#include <sstream>
#ifndef NDEBUG
#include <cassert>
#endif

namespace cdnalizerd {

using strings = std::vector<std::string>;

/// Holds the main part of the configuration data
struct Data {
    strings usernames;
    strings apikeys;
    strings regions;
    strings containers;
};

/// Holds a single mapping from a local folder to remote location
class ConfigEntry {
private:
    friend class Config;
    const Data* data;
    size_t username_index;
    size_t apikey_index;
    size_t region_index;
    size_t container_index;
    bool _snet;
    std::string _local_dir;
    std::string _remote_dir;
public:
  ConfigEntry(const Data *data, size_t username, size_t apikey, size_t region,
              size_t container, std::string local_dir, std::string remote_dir,
              bool snet)
      : data(data), username_index(username), apikey_index(apikey),
        region_index(region), container_index(container), _local_dir(local_dir),
        _remote_dir(remote_dir), _snet(snet) {}

    // Attribute Access
    const std::string& username() const { return data->usernames[username_index]; }
    const std::string& apikey() const { return data->apikeys[apikey_index]; }
    const std::string& region() const { return data->regions[region_index]; }
    const std::string& container() const { return data->containers[container_index]; }
    const std::string& local_dir() const { return _local_dir; }
    const std::string& remote_dir() const { return _remote_dir; }
    bool snet() const { return _snet; }

    // To allow easy sorting
    bool operator <(const ConfigEntry& other) const {
        return _local_dir < other._local_dir;
    }
    bool operator <(const std::string& path) const {
        return _local_dir < path;
    }
};

#ifndef NDEBUG
/// For debugging, make sure that we never use the list of Entries unsorted.
using EntriesBase = std::vector<ConfigEntry>;
struct Entries : public EntriesBase {
    bool dirty=true; /// true= list is unsorted
    void push_back(const value_type& val) {
        dirty = true;
        EntriesBase::push_back(val);
    }
    void push_back(value_type&& val) {
        dirty = true;
        EntriesBase::push_back(val);
    }
};
#else
using Entries = std::vector<ConfigEntry>
#endif

/// Holds our configuration
class Config : private Data {
private:
    bool snet = false;
    Entries _entries;
public:
    void addUsername(std::string username) { usernames.push_back(username); }
    void addApikey(std::string apikey) { apikeys.push_back(apikey); }
    void addRegion(std::string region) { regions.push_back(region); }
    void addContainer(std::string container) { containers.push_back(container); }
    void setSNet(bool new_val) { snet = new_val; } /// Toggles service net on and off
    void addEntry(std::string local_dir, std::string remote_dir) {
        if ((usernames.size() == 0) ||
            (apikeys.size() == 0) ||
            (regions.size() == 0) ||
            (containers.size() == 0))
            throw ConfigError("Need to have a valid username, apikey, region and container before adding a path pair");
        _entries.push_back({this, usernames.size()-1, apikeys.size()-1, regions.size()-1, containers.size()-1, local_dir, remote_dir, snet});
    }

    Config() = default;

    Config(const Config& other) : Data(other), _entries(other._entries) {
        // Update our entries to recognize us as the parent now
        for (auto& e : _entries)
            e.data = this;
    }

    Config(Config&& other) : Data(other), _entries(other._entries) {
        // Update our entries to recognize us as the parent now
        for (auto& e : _entries)
            e.data = this;
        other._entries.clear();
    }

    Config& operator=(const Config& other) {
        Data::operator=(other);
        _entries = other._entries;
        for (auto& e : _entries)
            e.data = this;
        return *this;
    }


    /// Makes the config ready for use
    const Config& use() {
        std::sort(_entries.begin(), _entries.end());
        #ifndef NDEBUG
        _entries.dirty = false;
        #endif
        return *this;
    }

    const ConfigEntry& getEntryByPath(const std::string& path) const {
        #ifndef NDEBUG
        assert(!_entries.dirty);
        #endif
        auto found = std::lower_bound(_entries.cbegin(), _entries.cend(), path);
        if (found != _entries.cend())
            return *found;
        else {
            std::stringstream msg;
            msg << "Couldn't find "
                << path << " in this list: ";
            for (auto& e : _entries)
                msg << e.local_dir() << ", ";
            throw std::logic_error(msg.str());
        }
    }
    const Entries& entries() const { return _entries; }
};

}
