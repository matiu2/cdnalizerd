#pragma once

#include "../config_reader/config.hpp"
#include "../AccountCache.hpp"
#include "../WorkerManager.hpp"

namespace cdnalizerd {
namespace processes {

void syncAllDirectories(yield_context &yield, const AccountCache &rs,
                        const Config &config, WorkerManager& workers);

} /* processes */ 
} /* cdnalizerd  */ 
