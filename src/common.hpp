#pragma once

#include <memory>
#include <string>
#include <boost/asio/spawn.hpp>

namespace cdnalizerd {

using sstring = std::shared_ptr<std::string>;
using boost::asio::yield_context;

  
} /* cdnalizerd  */ 
