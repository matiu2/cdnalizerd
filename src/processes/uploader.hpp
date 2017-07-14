#pragma once

#include "../config_reader/config.hpp"
#include "../Status.hpp"

namespace cdnalizerd {
namespace processes {

/// This function watches status.jobsToDo and does them until there are none left
void uploader(yield_context yield, Worker& worker);

}
} /* processes */ 

