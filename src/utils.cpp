#include "utils.hpp"

#include <sys/stat.h>
#include <dirent.h>
#include <cassert>
#include <cerrno>
#include <system_error>

namespace cdnalizerd {

bool isDir(const char *path) {
  struct stat sb;
  return (stat(path, &sb) == 0) && (S_ISDIR(sb.st_mode));
}

bool isDir(dirent *entry) {
  bool is_dir = entry->d_type == DT_DIR;
  // Some filesystmes need to use stat to find out if it's a dir or not
  if (!is_dir && (entry->d_type == DT_UNKNOWN))
    is_dir = isDir(entry->d_name);
  return is_dir;
}

void walkDir(const char *path, std::function<void(const char *)> callback) {
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
      if ((strcmp(entry->d_name, ".") == 0) &&
          (strcmp(entry->d_name, "..") == 0)) {
        callback(entry->d_name);
        walkDir(entry->d_name, callback);
      }
    }
  }
}

std::string joinPaths(const std::string &base, const std::string &extra) {
  auto out = base;
  if (out.back() != '/')
    out.append("/");
  if ((extra.front() == '/') && extra.size() > 1)
    std::copy(extra.begin() + 1, extra.end(), std::back_inserter(out));
  else
    out.append(extra);
  return out;
}

std::string unJoinPaths(const std::string base, const std::string &extra) {
  if ((base.size() <= extra.size()) &&
      (std::equal(base.begin(), base.end(), extra.begin()))) {
    return extra.substr(base.size());
  }
  // Nothing to return, because base is not a prefix of extra
  return "";
}

}
