#pragma once

#include "../Job.hpp"
#include <boost/exception/exception.hpp>
#include <boost/exception/errinfo_errno.hpp>
#include <string>

namespace cdnalizerd {

namespace jobs {

using http_code = boost::error_info<struct HTTPCode, unsigned short>;
using http_body = boost::error_info<struct HTTPBody, std::string>;

struct UploadError : virtual boost::exception {};

/// Upload a file
Job makeUploadJob(fs::path source, URL dest);

/// Upload a file, but first compare the MD5 sum
Job makeConditionalUploadJob(fs::path source, URL dest);

} /* jobs */ 
} /* cdnalizerd  */ 
