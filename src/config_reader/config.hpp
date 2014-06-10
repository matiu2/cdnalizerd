#pragma once

#include <algorithm>
#include <vector>
#include <string>
#include <sstream>

namespace cdnalizerd {

using strings = std::vector<std::string>;

/// Holds a single mapping from a local folder to remote location
class ConfigEntry {
private:
    std::string& _username;
    std::string& _apikey;
    std::string& _container;
    std::string _local_dir;
    std::string _remote_dir;
public:
    ConfigEntry(std::string& username, std::string& apikey, std::string& container, std::string local_dir, std::string remote_dir)
    : _username(username), _apikey(apikey), _container(container), _local_dir(local_dir), _remote_dir(remote_dir) {}
    // Attribute access
    const std::string& username() const { return _username; };
    const std::string& apikey() const { return _apikey; };
    const std::string& container() const { return _container; };
    const std::string local_dir() const { return _local_dir; };
    const std::string remote_dir() const { return _remote_dir; };
    
    // To allow easy sorting
    bool operator <(const ConfigEntry& other) const {
        return _local_dir < other._local_dir;
    }
    bool operator <(const std::string& path) const {
        return _local_dir < path;
    }

    // Assignment Operator so that vector sort will work
    ConfigEntry& operator =(const ConfigEntry& other) {
        _username = other._username;
        _apikey = other._apikey;
        _container = other._container;
        _local_dir = other._local_dir;
        _remote_dir = other._remote_dir;
        return *this;
    }
};

/// Used while the vectors are still in flux and iterators may become invalidated
struct TempEntry {
    size_t username_index;
    size_t apikey_index;
    size_t container_index;
    std::string path1;
    std::string path2;
};

/// Holds our configuration
class Config {
private:
    strings usernames;
    strings apikeys;
    strings containers;
    std::vector<ConfigEntry> entries;
public:
    Config(strings&& usernames, strings&& apikeys, strings&& containers, std::vector<TempEntry>&& input) : usernames(usernames), apikeys(apikeys), containers(containers) {
        entries.reserve(input.size());
        for (auto&& i : input) {
            ConfigEntry e{
                usernames.at(i.username_index),
                apikeys.at(i.apikey_index),
                containers.at(i.container_index),
                std::move(i.path1),
                std::move(i.path2)
            };
            entries.push_back(e);
        }
        // Sort the entries
        std::sort(entries.begin(), entries.end());
    }
    const ConfigEntry& getEntryByPath(const std::string& path) const {
        auto found = std::lower_bound(entries.cbegin(), entries.cend(), path);
        if (found != entries.cend())
            return *found;
        else {
            std::stringstream msg;
            msg << "Couldn't find "
                << path << " in this list: ";
            for (auto& e : entries)
                msg << e.local_dir() << ", ";
            throw std::logic_error(msg.str());
        }
    }
};

}
