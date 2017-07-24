#pragma once

#include "../config_reader/config.hpp"

namespace cdnalizerd {
namespace processes {

/// This is the main function in the app and watches for file changes, then launches processes as required
void watchForFileChanges(yield_context yield, const Config& config);

}
} /* processes */ 
