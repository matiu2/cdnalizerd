#pragma once

#include "exception_tags.hpp"
#include "logging.hpp"

#include <string>
#include <vector>

namespace cdnalizerd {

struct UnParsedURL {
public:
  std::vector<std::string> parts;
  UnParsedURL(std::string a, std::string b)
      : parts{{std::move(a)}, {std::move(b)}} {}
  UnParsedURL(UnParsedURL &&other, std::string next)
      : parts(std::move(other.parts)) {
    parts.push_back(std::move(next));
  }
};

struct URL {

private:
  std::string raw;
  void parse();

public:
  URL() : raw() {}
  URL(std::string raw) : raw(std::move(raw)) { parse(); }
  URL(UnParsedURL in);
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

/// Takes a filename or path and url encodes it
std::string urlencode(const std::string &path);

inline UnParsedURL operator/(UnParsedURL url, std::string s) {
  return UnParsedURL(std::move(url), urlencode(s));
}

inline UnParsedURL operator/(const URL &url, std::string s) {
  return UnParsedURL(url.whole(), urlencode(s));
}

} /* cdnalizerd  */ 
