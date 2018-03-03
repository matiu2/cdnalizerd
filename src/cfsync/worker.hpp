#pragma once

#include "../AccountCache.hpp"
#include "worker.hpp"

using namespace cdnalizerd;

enum JobType { upload, download, server_copy, local_copy };

/// Actually do the sync work

void cfsync(yield_context &yield, JobType jobType, ConfigEntry &entry) {
  // Account login information
  AccountCache accounts;
  auto found = accounts.find(entry.username);
  if (found == accounts.end())
    BOOST_THROW_EXCEPTION(boost::enable_error_info(std::runtime_error(
                              "All Rackspace accounts should be initialized "
                              "by the time this is called"))
                          << err::username(entry.username));
  Rackspace &rs = found->second;
  // Do the work
  URL url(rs.getURL(entry.region, entry.snet));

};
