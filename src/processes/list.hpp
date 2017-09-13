#pragma once

#include "../AccountCache.hpp"
#include "../config_reader/config.hpp"

#include <json.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/coroutine2/coroutine.hpp>

#include <string>

namespace cdnalizerd {

using namespace std::literals::string_literals;
using nlohmann::json;

struct ContainerEntry {
  const std::string& username;
  const std::string& container;
  std::vector<std::string> files;
};

using ListContainerCoroutine = boost::coroutines::coroutine<std::vector<std::string>>;
using ListContainerResult = ListContainerCoroutine::pull_type;
using ListContainerOut = ListContainerCoroutine::push_type;

using JSONListContainerCoroutine = boost::coroutines::coroutine<json>;
using JSONListContainerResult = JSONListContainerCoroutine::pull_type;
using JSONListContainerOut = JSONListContainerCoroutine::push_type;

/// Fills 'out' with the contents of a container. Returns true if we need to
void doListContainer(ListContainerOut &out, yield_context &yield,
                     const Rackspace &rs, const ConfigEntry &entry);

/// Fills 'out' with the contents of a container. Returns true if we need to
/// if 'restrict' is true, it'll only list paths in the prefix entry.remote_dir
void JSONDoListContainer(JSONListContainerOut &out, yield_context &yield,
                         const Rackspace &rs, const ConfigEntry &entry,
                         bool restrict);

namespace processes {

inline ListContainerResult listContainer(yield_context &yield,
                                         const Rackspace &rs,
                                         const ConfigEntry &entry) {
  return ListContainerResult([&](ListContainerOut &pusher) {
    doListContainer(pusher, yield, rs, entry);
  });
}

inline JSONListContainerResult JSONListContainer(yield_context &yield,
                                                 const Rackspace &rs,
                                                 const ConfigEntry &entry,
                                                 bool restrict_to_remote_dir) {
  return JSONListContainerResult([&](JSONListContainerOut &pusher) {
    JSONDoListContainer(pusher, yield, rs, entry, restrict_to_remote_dir);
  });
}

/// Prints out the contents of all containers
void listContainers(yield_context yield, const AccountCache& accounts, const Config &config);

/// Prints out the contents of all containers
void JSONListContainers(yield_context yield, const AccountCache& accounts, const Config &config);


} /* processes { */ 
} /* cdnalizerd  */
