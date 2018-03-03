#pragma once
/// Gets auth tokens for all the accounts

#include "common.hpp"
#include "Rackspace.hpp"
#include "config/config.hpp"

#include <functional>

namespace cdnalizerd {

/// maps config.username to a Rackspace object
using AccountCache = std::map<std::string, Rackspace>;

/// Fills a single account cache entry from a single ConfigEntry
void fillSingleAccountCache(yield_context &yield,
                            const ConfigEntry &configEntry,
                            AccountCache &cache);

/// Worker that fills an account cache by logging on to all the RS accounts
/// and getting the token and json
/// Calls 'onDone' once all the logins are complete.
void fillAccountCache(yield_context &yield, const Config &config,
                      AccountCache &cache, std::function<void()> onDone);

} /* cdnalizerd  */ 
