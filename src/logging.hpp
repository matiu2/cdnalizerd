#pragma once

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

namespace cdnalizerd {

namespace logging = boost::log;

void initLogging() {
  //logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::info);
}

} /* cdnalizerd */ 
