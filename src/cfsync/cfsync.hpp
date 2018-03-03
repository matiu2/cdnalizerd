#pragma once

#include "../AccountCache.hpp"

#include <cassert>

using namespace cdnalizerd;

enum JobType { upload, download, server_copy, local_copy };

/// Actually do the sync work

void cfsync(yield_context &yield, JobType jobType, ConfigEntry &entry);
