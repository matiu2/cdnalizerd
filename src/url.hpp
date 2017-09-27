#pragma once

#include "exception_tags.hpp"
#include "logging.hpp"

#include <string>

namespace cdnalizerd {

struct UnParsedURL {
public:
  std::string raw;
  UnParsedURL(std::string raw) : raw(raw) {}
};

struct URL {

private:
  std::string raw;
  void parse();

public:
  URL() : raw() {}
  URL(std::string raw) : raw(std::move(raw)) { parse(); }
  URL(UnParsedURL in) : raw(std::move(in.raw)) { parse(); }
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
};

UnParsedURL joinStrings(const std::string& a, const std::string& b);

inline UnParsedURL operator/(const UnParsedURL &url, std::string s) {
  return joinStrings(url.raw, s);
}

inline UnParsedURL operator/(const URL &url, std::string s) {
  return joinStrings(url.whole(), s);
}


} /* cdnalizerd  */ 
