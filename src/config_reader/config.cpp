#include "config.hpp"

#include "../logging.hpp"

namespace cdnalizerd {

bool ConfigEntry::shouldIgnoreFile(const std::string& fileName) const {
  // Search through each list
  if (shouldIgnoreDirectory(fileName))
    return true;
  for (const std::regex& regex : filesToIgnore)
    if (std::regex_search(fileName, regex, std::regex_constants::match_any))
      return true;
  return false;
}

bool ConfigEntry::shouldIgnoreDirectory(const std::string& dirName) const {
  // Search through each list
  for (const std::regex& regex : directoriesToIgnore)
    if (std::regex_search(dirName, regex, std::regex_constants::match_any)) {
      DLOG_S(9) << "Ignoring directory: " << dirName;
      return true;
    }
  DLOG_S(9) << "Not ignoring directory: " << dirName;
  return false;
}


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

const ConfigEntry& Config::getEntryByPath(const std::string &path) const {
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
