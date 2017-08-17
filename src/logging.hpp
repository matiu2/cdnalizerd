#pragma once

#include <boost/log/core.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/trivial.hpp>

namespace cdnalizerd {

namespace logging = boost::log;
using logger = boost::log::sources::logger;

enum severity_level { trace, debug, info, warning, error, fatal };

void init_logging();

logger& defaultLogger();

} /* cdnalizerd */ 
