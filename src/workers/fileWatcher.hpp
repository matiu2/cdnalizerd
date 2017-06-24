#pragma once

#include <vector>

#include "../config_reader/config.hpp"
#include "../Status.hpp"

namespace cdnalizerd {
namespace workers {

/// This is the main function in the app and watches for file changes, then launches workers as required
void watchForFileChanges(yield_context &yield, Status &status,
                         const Config &config);

}
} /* workers */ 
