#include "utils.hpp"

#include <sys/stat.h>
#include <dirent.h>
#include <cassert>
#include <cerrno>
#include <system_error>

namespace cdnalizerd {

std::string getContainerUrl(const Rackspace &login, const ConfigEntry &config) {
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

bool isDir(const char *path) {
  struct stat sb;
  return (stat(path, &sb) == 0) && (S_ISDIR(sb.st_mode));
}

bool isDir(dirent* entry) {
  bool is_dir = entry->d_type == DT_DIR;
  // Some filesystmes need to use stat to find out if it's a dir or not
  if (!is_dir && (entry->d_type == DT_UNKNOWN))
    is_dir = isDir(entry->d_name);
  return is_dir;
}

void walkDir(const char* path, std::function<void(const char*)> callback) {
  assert(isDir(path));
  auto dir = opendir(path);
  if (!dir)
    throw std::system_error(errno, std::system_category());
  // Read through this directory
  while (true) {
    dirent *entry = readdir(dir);
    if (entry == nullptr)
      break;
    if (isDir(entry)) {
      if ((strcmp(entry->d_name, ".") == 0) && (strcmp(entry->d_name, "..") == 0)) {
        callback(entry->d_name);
        walkDir(entry->d_name, callback);
      }
    }
  }
}

}

