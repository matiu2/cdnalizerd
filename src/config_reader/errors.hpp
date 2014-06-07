#pragma once

#include <stdexcept>

namespace cdnalizerd {

/// Thrown when we can't read the configuration
struct ConfigError : std::logic_error {};

/// When we couldn't parse a quoted string
struct ParseError : ConfigError {};

}
