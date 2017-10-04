#pragma once

#include "../Job.hpp"
#include "../url.hpp"

namespace cdnalizerd {
namespace jobs {

/// Returns a job that will wipe a file from the destination server
Job makeRemoteDeleteJob(URL dest);

} /* jobs */
} /* cdnalizerd  */
