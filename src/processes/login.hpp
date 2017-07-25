#pragma once

#include "../AccountCache.hpp"
#include "../config_reader/config.hpp"

namespace cdnalizerd {

// Fills the cache of all accounts - spawns more cooperative threads and waits for them
void login(yield_context &yield, AccountCache& accounts, const Config& config);

} /* cdnalizerd */ 
