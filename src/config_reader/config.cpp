#include "config.hpp"

namespace cdnalizerd {

void Config::addEntry(std::string local_dir, std::string remote_dir) {
  // Validate what we have
  if ((!lastEntry.username) || (lastEntry.username->empty()) ||
      (!lastEntry.apikey) || (lastEntry.apikey->empty()) ||
      (!lastEntry.region) || (lastEntry.region->empty()) ||
      (!lastEntry.container) || (lastEntry.container->empty()))
    throw ConfigError("Need to have a valid username, apikey, region "
                      "and container before adding a path pair");
  // Now create it
  ConfigEntry result(lastEntry);
  result.local_dir = std::move(local_dir);
  result.remote_dir = std::move(remote_dir);
  // Place it in a sorted location
  auto place =
      std::lower_bound(_entries.begin(), _entries.end(), result.local_dir);
  _entries.emplace(place, std::move(result));
}

ConfigEntry Config::getEntryByPath(const std::string &path) const {
  auto found = std::lower_bound(_entries.begin(), _entries.end(), path);
  if (found != _entries.end())
    return *found;
  else {
    std::stringstream msg;
    msg << "Couldn't find " << path << " in this list: ";
    for (auto &e : _entries)
      msg << e.local_dir << ", ";
    throw std::logic_error(msg.str());
  }
}


} /* cdnalazired */ 
