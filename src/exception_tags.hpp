#pragma once
/// here we define boost::exception tags to add context to exceptions
/// http://www.boost.org/doc/libs/1_64_0/libs/exception/doc/exception_types_as_simple_semantic_tags.html

#include <boost/beast/http/status.hpp>
#include <boost/exception/enable_error_info.hpp>
#include <boost/exception/exception.hpp>
#include <boost/exception/info.hpp>
#include <boost/system/error_code.hpp>

#include <string>

namespace cdnalizerd {
namespace err {

// ** Tags for the main process ** //
using username = boost::error_info<struct UserName, std::string>;

// ** Tags for the main process - END ** //

// ** Tags to do with the uplodaing files jobs ** //

// What we were doing when an error occured
using action = boost::error_info<struct Action, std::string>;
using source = boost::error_info<struct Source, std::string>;
using destination = boost::error_info<struct Destination, std::string>;
using position = boost::error_info<struct Position, size_t>;
using error_code =
    boost::error_info<struct ErrorCode, boost::system::error_code>;
using error_category =
    boost::error_info<struct ErrorCategory, boost::system::error_category>;
using http_status =
    boost::error_info<struct HTTPStatus, boost::beast::http::status>;

using URL = boost::error_info<struct Original, std::string>;

// ** Tags to do with the uplodaing files jobs - ENDS ** //
  
} /* err  */ 
  
} /* cdnalizerd  */ 

