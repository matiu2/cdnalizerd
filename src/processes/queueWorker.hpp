#pragma once

#include "../config_reader/config.hpp"
#include "../Status.hpp"

namespace cdnalizerd {
namespace processes {

/// This function watches status.jobsToDo and does them until there are none left
void queueWorker(yield_context &yield, Status& status, Worker& worker);
}
} /* processes */ 

