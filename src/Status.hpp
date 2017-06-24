#pragma once

#include <map>
#include <vector>
#include <memory>

#include "Rackspace.hpp"
#include "inotify.hpp"

namespace cdnalizerd {
  
/// Status of the whole app; one instance will be shared by all the workers
struct Status {
  std::map<std::string, std::unique_ptr<Rackspace>> logins; // maps username to login
  /// Holds file move operations that are waiting for a pair
  std::map<uint32_t, inotify::Event> cookies;
  // TODO: Add status of all file uploads and ongoing operations
};

} /* cdnalizerd  */ 

