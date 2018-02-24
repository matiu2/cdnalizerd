/// Bits and pieces that we need
#pragma once

#include <boost/hana.hpp>

#include "common.hpp"
#include "Rackspace.hpp"
#include "config/config.hpp"

namespace cdnalizerd {

/// Returns true if 'path' is a directory
bool isDir(const char *path);

/// Joins two urls with exactly one path separator
template <typename... T>
std::string joinPaths(T...aParams) {
  auto params = boost::hana::make_tuple(aParams...);
  return boost::hana::fold(params, [](std::string a, const std::string &b) {
    if (((!a.empty()) && (a.back() == '/')) ||
        ((!b.empty()) && (b.front() == '/')))
      return a + b;
    else
      return a + '/' + b;
  });
}

/// When 'extra' starts with 'base', returns the bit after 'base' (with no slashes)
std::string unJoinPaths(const std::string base, const std::string &extra);

}
