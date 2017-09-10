#include "upload.hpp"

#include "../logging.hpp"
#include "../exception_tags.hpp"

#include <boost/beast/core/file.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/exception/enable_error_info.hpp>
#include <boost/exception/exception.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/throw_exception.hpp>
#include <boost/utility/string_view.hpp>
#include <iomanip>
#include <iostream>
#include <openssl/md5.h>
#include <sstream>
#include <string>

using namespace std::literals;

namespace cdnalizerd {
namespace jobs {

/// Returns the file size and md5
const std::string md5_from_file(const fs::path &path) {
  unsigned char result[MD5_DIGEST_LENGTH];
  try {
    assert(fs::is_regular_file(path));
    boost::iostreams::mapped_file_source src(path.native());
    MD5((const unsigned char *)src.data(), src.size(), result);
  } catch (std::exception e) {
    BOOST_THROW_EXCEPTION(boost::enable_error_info(e)
                          << err::action("Taking md5"));
  } catch (...) {
    BOOST_THROW_EXCEPTION(
        boost::enable_error_info(std::runtime_error("Unknown exception"))
        << err::action("Taking md5"));
  }

  std::ostringstream sout;
  sout << std::hex << std::setfill('0');
  for (auto c : result)
    sout << std::setw(2) << (int)c;

  return sout.str();
}

/// Uploads a file.
/// if md5 is not set, it'll read it from the file
void upload(const fs::path &source, const URL &dest, HTTPS &conn,
            const std::string &token, std::string md5 = "") {
  try {
    LOG_SCOPE_F(5, "cdnalizerd::upload");
    LOG_S(INFO) << "Uploading " << source.native() << " to " << dest.whole();
    namespace http = boost::beast::http;
    using boost::beast::file_mode;
    if (md5.empty()) {
      DLOG_S(5) << "Getting MD5 of local file: " << source.native();
      md5 = md5_from_file(source);
    }
    // Make the upload request
    http::request<http::file_body> req;
    setDefaultHeaders(req, token);
    req.set(http::field::etag, md5);
    req.target(dest.path_part());
    req.method(http::verb::put);
    // Open the file
    boost::system::error_code ec;
    req.body.open(source.native().c_str(), boost::beast::file_mode::scan, ec);
    if (ec != boost::system::errc::success) {
      BOOST_THROW_EXCEPTION(
          boost::enable_error_info(boost::system::system_error(ec))
          << err::action("Openning file"));
    }
    DLOG_S(9) << "HTTP Request: " << req.base();
    http::async_write(conn.stream(), req, conn.yield);
    // Make sure it's OK
    http::response<http::empty_body> response;
    http::async_read(conn.stream(), conn.read_buffer, response, conn.yield);
    DLOG_S(9) << "HTTP Response: " << response;
    if (response.result() != http::status::ok) {
      BOOST_THROW_EXCEPTION(
          boost::enable_error_info(std::runtime_error("HTTP Bad Response"))
          << err::http_status(response.result()));
    }
  } catch (boost::exception &e) {
    e << err::action("Uploading") << err::source(source.native())
      << err::destination(dest.whole());
    throw;
  } catch (std::exception &e) {
    throw boost::enable_error_info(e) << err::source(source.native())
                                      << err::destination(dest.whole());
  }
};

Job makeUploadJob(fs::path source, URL dest) {
  Job::Work go = [source, dest](HTTPS &conn, const std::string &token) {
    upload(source, dest, conn, token);
  };
  return Job("Upload "s + source.string() + " to " + dest.whole(), go);
}

Job makeConditionalUploadJob(fs::path source, URL dest) {
  Job::Work go = [source, dest](HTTPS &conn, const std::string &token) {
    LOG_SCOPE_F(5, "cdnalizerd::conditionalUpload");
    LOG_S(INFO) << "Conditionally Uploading " << source.native() << " to "
                << dest.whole();
    try {
      // Get the MD5 of the local file
      DLOG_S(5) << "Getting MD5 of local file: " << source.native();
      std::string md5;
      md5 = md5_from_file(source);
      // Get the MD5 of the remote file
      DLOG_S(5) << "Requesting URL: " << dest.whole() << std::endl;
      // Get the MD5 of the existing file
      http::request<http::empty_body> req;
      req.set(http::field::user_agent, "cdnalizerd v0.2");
      req.set(http::field::accept, "application/json");
      req.set("X-Auth-Token", token);
      req.method(http::verb::head);
      http::async_write(conn.stream(), req, conn.yield);
      // Get the response
      http::response<http::empty_body> response;
      http::async_read(conn.stream(), conn.read_buffer, response, conn.yield);
      DLOG_S(9) << "HTTP Response: " << response;
      if (response.result() != http::status::ok) {
        BOOST_THROW_EXCEPTION(
            boost::enable_error_info(std::runtime_error("HTTP Bad Response"))
            << err::http_status(response.result()));
      }
      auto etag = response.find(http::field::etag);
      boost::string_view serverMD5(response[http::field::etag]);

      // Now compare the md5 from the server with the file's actual md5
      if (md5 != serverMD5) {
        // Upload the file then
        upload(source, dest, conn, token, md5);
      }
    } catch (std::exception &e) {
      using namespace std;
      LOG_S(ERROR) << "std::exception trying to conditionally upload "
                   << source.native() << " to " << dest.whole() << ": "
                   << e.what() << std::endl;
    } catch (boost::exception &e) {
      LOG_S(ERROR) << "Unable to conditionally upload " << source.native()
                   << " to " << dest.whole() << ": "
                   << boost::diagnostic_information(e, true) << std::endl;
    } catch (...) {
      LOG_S(ERROR) << "Unable to conditionally upload " << source.native()
                   << " to " << dest.whole() << ": "
                   << boost::current_exception_diagnostic_information(true)
                   << std::endl;
    }
  };
  return Job("Conditional upload "s + source.string() + " to " + dest.whole(),
             go);
}

} /* jobs */
} /* cdnalizerd  */
