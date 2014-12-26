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
#include "inotify-cxx.h"

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
  const Config& config;
  std::vector<WatchGroup> groups;
  Inotify notify;
  void readConfig(const Config& config) {
    for (const auto &entry : config.entries()) {
      Rackspace& login = logins[entry.username()];
      if (!login)
        login.login(entry.username(), entry.apikey());
      InotifyWatch watch(entry.local_dir(), IN_ALL_EVENTS | IN_CLOSE_WRITE |
                                                IN_DELETE | IN_MOVED_FROM |
                                                IN_MOVED_TO);
      notify.Add(watch);
    }
  }
  std::string getContainerUrl(const Rackspace& login, const ConfigEntry& config) const {
    assert(login); // Can't watch a dir until we're logged in to rackspace
    const json::JList &regions =
        login.response.at("access").at("serviceCatalog").at("cloudFiles").at(
            "endPoints");
    std::string url;
    // We get a list of regions, it doesn't come as a json dict, so we have to
    // scan through them to find our configured region
    for (const auto &region : regions) {
      if (region.at("region") == config.region()) {
        if (config.snet())
          url = region.at("privateURL");
        else
          url = region.at("publicURL");
        break;
      }
    }
    if (url.empty())
      throw std::runtime_error(
          std::string("Unable to find url for cloud files region: ") +
          config.region());
    auto join_to_url = [&url](const std::string &extra) {
      if (url.back() != '/')
        url.append("/");
      url.append(extra);
      if (url.back() != '/')
        url.append("/");
      return url;
    };
    join_to_url(config.container());
    join_to_url(config.remote_dir());
    return url;
  }
public:
  Watcher(Config &config) : config(config.use()) {
    }
};

}
