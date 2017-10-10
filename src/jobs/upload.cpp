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
std::string md5_from_file(const fs::path &path) {
  LOG_SCOPE_F(5, "md5_from_file");
  unsigned char result[MD5_DIGEST_LENGTH];
  try {
    if (!fs::is_regular_file(path)) {
      LOG_S(0) << "File may have been removed since event happened. Upload aborted";
      return "";
    }
    boost::iostreams::mapped_file_source src(path.native());
    MD5((const unsigned char *)src.data(), src.size(), result);
  } catch (std::exception &e) {
    LOG_S(WARNING) << boost::diagnostic_information(e, true);
    BOOST_THROW_EXCEPTION(boost::enable_error_info(e)
                          << err::action("Taking md5"));
  } catch (...) {
    LOG_S(WARNING) << boost::current_exception_diagnostic_information(true);
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
void upload(const fs::path &source, URL dest, HTTPS &conn,
            const std::string &token, std::string md5 = "") {
  try {
    LOG_SCOPE_F(5, "cdnalizerd::upload");
    if (fs::file_size(source) == 0) {
      // Code shouldn't really get here, because there's another check in
      // processes/mainProcess.cpp where it'll only create this job if the file
      // size is > 0
      LOG_S(WARNING) << "Unable to upload zero length file. Aborting: " << source.native();
      return;
    }
    LOG_S(INFO) << "Uploading " << source.native() << " to " << dest.whole();
    namespace http = boost::beast::http;
    using boost::beast::file_mode;
    if (md5.empty() && (source.size() > 0)) {
      DLOG_S(8) << "Getting MD5 of local file: " << source.native();
      md5 = md5_from_file(source);
      DLOG_S(9) << "Local MD5: " << md5;
    }
    // Make the upload request
    http::request<http::file_body> req;
    setDefaultHeaders(req, token);
    req.set(http::field::etag, md5);
    req.set(http::field::host, dest.host);
    req.target(dest.pathAndSearch);
    req.method(http::verb::put);
    // Open the file
    boost::system::error_code ec;
    req.body.open(source.native().c_str(), boost::beast::file_mode::scan, ec);
    if (ec != boost::system::errc::success) {
      BOOST_THROW_EXCEPTION(
          boost::enable_error_info(boost::system::system_error(ec))
          << err::action("Openning file"));
    }
    req.set(http::field::content_length, req.body.size());
    LOG_S(9) << "HTTP Request: " << req.base();
    http::async_write(conn.stream(), req, conn.yield);
    // Make sure it's OK
    http::response<http::string_body> response;
    http::async_read(conn.stream(), conn.read_buffer, response, conn.yield);
    LOG_S(9) << "HTTP Response: " << response;
    switch (response.result()) {
    case http::status::accepted: {
      LOG_S(0) << "Upload has been accepted for processing by the server";
      break;
    }
    case http::status::created: {
      LOG_S(0) << "Upload Successful";
      break;
    }
    case http::status::unauthorized: {
      // TODO: Maybe re-authorize and try again
      LOG_S(ERROR) << "Upload Failed - Unauthorized";
      break;
    }
    case http::status::length_required: {
      LOG_S(ERROR) << "Upload Failed - Length required";
      break;
    }
    case http::status::unprocessable_entity: {
      LOG_S(ERROR) << "Upload Failed - Un processable entity";
      break;
    }
    default:
      BOOST_THROW_EXCEPTION(
          boost::enable_error_info(std::runtime_error("HTTP Bad Response"))
          << err::http_status(response.result()));
    };
  } catch (boost::system::system_error &e) {
    auto e2 = boost::enable_error_info(e) << err::source(source.native())
                                          << err::destination(dest.whole());
    LOG_S(ERROR) << "System error: " << e.code().value() << " - "
                 << e.code().message() << " - " << e.code().category().name()
                 << " - " << boost::diagnostic_information_what(e2, true);
  } catch (boost::exception &e) {
    e << err::action("Uploading") << err::source(source.native())
      << err::destination(dest.whole());
    LOG_S(ERROR) << boost::diagnostic_information_what(e, true);
  } catch (std::exception &e) {
    auto e2 = boost::enable_error_info(e) << err::source(source.native())
                                          << err::destination(dest.whole());
    LOG_S(ERROR) << e.what() << " - "
                 << boost::diagnostic_information_what(e2, true);
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
      if (md5 == "")
        return;
      // Get the MD5 of the existing file from the server
      for (char ch : dest.path)
        std::cout << std::hex << ch;
      http::request<http::empty_body> req{http::verb::head, dest.path, 11};
      req.set(http::field::host, dest.host);
      req.set(http::field::user_agent, userAgent());
      req.set(http::field::accept, "application/json");
      req.set("X-Auth-Token", token);
      LOG_S(9) << "HTTP Request: " << req;
      http::async_write(conn.stream(), req, conn.yield);
      // Get the response
      http::response_parser<http::empty_body> parser;
      parser.skip(true);

      http::async_read_header(conn.stream(), conn.read_buffer, parser, conn.yield);
      auto response = parser.release();
      LOG_S(9) << "HTTP Response: " << response;
      if (response.result() == http::status::not_found) {
        // File doesn't exist on the server, upload it
        LOG_S(1) << "File not found on server, uploading..";
        upload(source, dest, conn, token, md5);
        return;
      } else if (response.result() != http::status::ok) {
        LOG_S(ERROR) << "Bad HTTP Response. HTTP Request: " << req
                     << "\n Response: " << response;
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
      LOG_S(ERROR) << e.what()
                   << " - std::exception trying to conditionally upload "
                   << source.native() << " to " << dest.whole() << ": "
                   << std::endl;
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
