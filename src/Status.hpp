#pragma once

#include <map>
#include <queue>
#include <list>
#include <memory>

#include <jsonpp11/json_class.hpp>

#include "Worker.hpp"
#include "config_reader/config.hpp"

namespace cdnalizerd {

using namespace std::string_literals;

/// Status of the whole app; one instance will be shared by all the workers
struct Status {
  std::map<std::string, std::pair<std::string, json::JSON>>
      logins; // maps username to {token, login_json} pair
};

} /* cdnalizerd  */ 
