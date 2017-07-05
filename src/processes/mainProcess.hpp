#pragma once

#include "../config_reader/config.hpp"
#include "../Status.hpp"

namespace cdnalizerd {
namespace processes {

/// This is the main function in the app and watches for file changes, then launches processes as required
void watchForFileChanges(yield_context &yield, Status &status,
                         const Config &config);

}
} /* processes */ 
