#pragma once
/** This compilation unit allows us to list files in remote rackspace servers.
 * The server paginates the output. Using asynchronous API and generators
 * (through coroutines), we just present the output as a single iterable list
 */

#include "../AccountCache.hpp"
#include "../config/config.hpp"

#include <nlohmann/json.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/coroutine2/coroutine.hpp>

#include <string>

namespace cdnalizerd {
namespace processes {

using namespace std::literals::string_literals;
using nlohmann::json;

/// A generator of individual strings or json dicts for the detailed view, which
/// are entries on the server This returns a single cointainer entry
using ListEntriesCoroutine = boost::coroutines::coroutine<std::string>;
using ListEntriesPusher = ListEntriesCoroutine::push_type;
using ListEntriesResult = ListEntriesCoroutine::pull_type;

/// A generator of json straight from the server
using DetailedListEntriesCoroutine =
    boost::coroutines::coroutine<nlohmann::json>;
using DetailedListEntriesPusher = DetailedListEntriesCoroutine::push_type;
using DetailedListEntriesResult = DetailedListEntriesCoroutine::pull_type;

/// Returns a generator of remote entries
ListEntriesResult listContainer(yield_context &yield, const Rackspace &rs,
                                const ConfigEntry &entry,
                                std::string extra_params = "");

/// Returns a generator of remote entries
DetailedListEntriesResult detailedListContainer(yield_context &yield,
                                                const Rackspace &rs,
                                                const ConfigEntry &entry,
                                                std::string extra_params = "");

} /* processes { */ 
} /* cdnalizerd  */
