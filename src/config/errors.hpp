#pragma once

#include <stdexcept>
#include <string>

namespace cdnalizerd {

/// Thrown when we can't read the configuration
struct ConfigError : std::logic_error {
  ConfigError(const char *msg) : std::logic_error(msg) {}
  ConfigError(std::string msg) : std::logic_error(msg) {}
};

/// When we couldn't parse a quoted string
struct ParseError : ConfigError {
  ParseError(const char *msg) : ConfigError(msg) {}
};
}
