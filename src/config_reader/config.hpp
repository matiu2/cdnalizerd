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
    strings containers;
};

/// A temoorary struct that holds references to all the data in a config Entry
struct EntryData {
    const std::string& username;
    const std::string& apikey;
    const std::string& container;
    const std::string& local_dir;
    const std::string& remote_dir;
};

/// Holds a single mapping from a local folder to remote location
class ConfigEntry {
private:
    size_t username_index;
    size_t apikey_index;
    size_t container_index;
    std::string local_dir;
    std::string remote_dir;
public:
    ConfigEntry(size_t username, size_t apikey, size_t container, std::string local_dir, std::string remote_dir)
    : username_index(username), apikey_index(apikey), container_index(container), local_dir(local_dir), remote_dir(remote_dir) {}

    EntryData read(const Data& data) const {
        return {
            data.usernames[username_index], 
            data.apikeys[apikey_index], 
            data.containers[container_index], 
            local_dir,
            remote_dir
        };
    }

    // To allow easy sorting
    bool operator <(const ConfigEntry& other) const {
        return local_dir < other.local_dir;
    }
    bool operator <(const std::string& path) const {
        return local_dir < path;
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
    Entries entries;
public:
    void addUsername(std::string username) { usernames.push_back(username); }
    void addApikey(std::string apikey) { apikeys.push_back(apikey); }
    void addContainer(std::string container) { containers.push_back(container); }
    void addEntry(std::string local_dir, std::string remote_dir) {
        if ((usernames.size() == 0) ||
            (apikeys.size() == 0) ||
            (containers.size() == 0))
            throw ConfigError("Need to have a valid username, apikey and container before adding a path pair");
        entries.push_back({usernames.size()-1, apikeys.size()-1, containers.size()-1, local_dir, remote_dir});
    }

    Config() = default;

    /// Makes the config ready for use
    const Config& use() {
        std::sort(entries.begin(), entries.end());
        #ifndef NDEBUG
        entries.dirty = false;
        #endif
        return *this;
    }
    EntryData getEntryByPath(const std::string& path) const {
        #ifndef NDEBUG
        assert(!entries.dirty);
        #endif
        auto found = std::lower_bound(entries.cbegin(), entries.cend(), path);
        if (found != entries.cend())
            return found->read(*this);
        else {
            std::stringstream msg;
            msg << "Couldn't find "
                << path << " in this list: ";
            for (auto& e : entries)
                msg << e.local_dir << ", ";
            throw std::logic_error(msg.str());
        }
    }
};

}
