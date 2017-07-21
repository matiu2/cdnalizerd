#pragma once

#include <istream>
#include <ostream>
#include <functional>

#include "config.hpp"

namespace cdnalizerd {

/** Reads in a config file.
 *
 **/
Config read_config(const std::string& filename);
}
