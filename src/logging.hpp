#pragma once

#define LOGURU_WITH_STREAMS 1
#include <loguru.hpp>

#define FATAL -3
#define ERROR -2
#define WARNING -1
#define INFO 0
#define DEBUG 5
#define TRACE 9

namespace cdnalizerd {
  
/// @verbosity - Maximum log output verbosity level --
///              -5=FATAL, 0=INFO, 5=DEBUG 9=TRACE
inline void init_logging(int verbosity) {
	loguru::g_stderr_verbosity = verbosity;
}

} /* cdnalizerd  */ 

