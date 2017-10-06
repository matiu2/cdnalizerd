#include "delete.hpp"

#include "../url.hpp"

using namespace std::literals;

namespace cdnalizerd {
namespace jobs {

void deleteRemoteFile(URL dest, HTTPS &conn, const std::string &token) {
  LOG_SCOPE_F(5, "Remote delete");
  http::request<http::empty_body> req{http::verb::delete_, dest.path, 11};
  req.set(http::field::host, dest.host);
  req.set(http::field::user_agent, userAgent());
  req.set(http::field::accept, "application/json");
  req.set("X-Auth-Token", token);
  DLOG_S(9) << "HTTP Request: " << req;
  http::async_write(conn.stream(), req, conn.yield);
  DLOG_S(9) << "Reading response";
  // Get the response
  http::response<http::empty_body> response;
  http::async_read(conn.stream(), conn.read_buffer, response, conn.yield);
  DLOG_S(9) << "HTTP Response: " << response;
  switch (response.result()) {
  case http::status::not_found: {
    LOG_S(WARNING) << "Tried to delete a file but it didn't exist: "
                << dest.whole();
    break;
  }
  case http::status::no_content: {
    LOG_S(0) << "Remote deletion successful: " << dest.whole();
    break;
  }
  default:
    BOOST_THROW_EXCEPTION(
        boost::enable_error_info(std::runtime_error("HTTP Bad Response"))
        << err::http_status(response.result()));
  };
}

Job makeRemoteDeleteJob(URL dest) {
  return Job("Remote delete job for "s + dest.whole(),
             std::bind(deleteRemoteFile, std::move(dest), std::placeholders::_1,
                       std::placeholders::_2));
}
    
} /* jobs */ 
} /* cdnalizerd  */ 
