#pragma once

#include "exception_tags.hpp"

#include <string>

namespace cdnalizerd {

struct URL {

private:
  std::string raw;
  void parse();

public:
  URL() : raw() {}
  URL(std::string raw) : raw(std::move(raw)) { parse(); }
  std::string scheme;
  std::string user;
  std::string password;
  std::string host;
  std::string port;
  std::string path;
  std::string search;
  std::string pathAndSearch;
  std::string scheme_host_port() {
    if (port.empty())
      return scheme + host;
    else
      return scheme + host + ':' + port;
  }
  const std::string &whole() const { return raw; }
  URL& operator/(std::string s) {
    bool urlEndsWithSlash = (!raw.empty()) && (raw.back() == '/');
    bool pathStartsWithSlash = (!path.empty()) && (path.front() == '/');
    if (urlEndsWithSlash && pathStartsWithSlash)
      return raw.substr(0, raw.size() - 1) + path;
    else if (urlEndsWithSlash || pathStartsWithSlash)
      return raw + path;
    else
      return raw + '/' + path;
  }
};

} /* cdnalizerd  */ 
