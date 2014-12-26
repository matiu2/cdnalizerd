/// Bits and pieces that we need
#pragma once

#include "login.hpp"
#include "config_reader/config.hpp"

namespace cdnalizerd {

std::string getContainerUrl(const Rackspace& login, const ConfigEntry& config);

}
