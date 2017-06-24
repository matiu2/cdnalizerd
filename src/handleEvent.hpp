#pragma once

#include "Status.hpp"
#include "inotify.hpp"

namespace cdnalizerd {

void handleEvent(inotify::Event &&event);
  
} /* cdnalizerd  */ 
