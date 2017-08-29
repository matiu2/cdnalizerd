#include "upload.hpp"

#include "../logging.hpp"

#include <openssl/md5.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/exception/exception.hpp>
#include <string>

using namespace std::literals;

namespace cdnalizerd {
namespace jobs {
    
const std::string md5_from_file(const fs::path& path) {
  unsigned char result[MD5_DIGEST_LENGTH];
  try {
    assert(fs::is_regular_file(path));
    boost::iostreams::mapped_file_source src(path.native());
    MD5((const unsigned char*)src.data(), src.size(), result);
  } catch (std::exception e)  {
    LOG_S(ERROR) << "Unable to get MD5 of file exception '" << path
                 << "': " << e.what() << std::endl;
  } catch (...) {
    LOG_S(ERROR) << "Unable to get MD5 of file '" << path
                 << "': UNKOWN EXCEPTION" << std::endl;
  }

  std::ostringstream sout;
  sout << std::hex << std::setfill('0');
  for (auto c : result)
    sout << std::setw(2) << (int)c;

  return sout.str();
}

Job makeUploadJob(fs::path source, URL dest) {
  Job::Work go = [source, dest](REST &conn) {
    // Get the MD5 of the local file
    DLOG_S(5) << "Getting MD5 of local file: " << source.native();
    std::string md5(md5_from_file(source));
    std::ifstream file(source.native());
    conn.put(dest.path_part()).add_header("ETag", md5).body(file).go();
  };
  return Job("Upload "s + source.string() + " to " + dest.whole(), go);
}

Job makeConditionalUploadJob(fs::path source, URL dest) {
  Job::Work go = [source, dest](REST &conn) {
    LOG_SCOPE_F(5, "cdnalizerd::conditionalUpload");
    LOG_S(INFO) << "Conditionally Uploading " << source.native() << " to "
                << dest.whole();
    try {
      // Get the MD5 of the local file
      DLOG_S(5) << "Getting MD5 of local file: " << source.native();
      std::string md5(md5_from_file(source));
      // Get the MD5 of the remote file
      DLOG_S(5) << "Requesting URL: " << dest.whole() << std::endl;
      RESTClient::http::Response response(
          conn.head(dest.path_part())
              .add_header("Accept", "application/json")
              .add_header("Accept-Encoding", "gzip, deflate")
              .go());
      if ((response.code < 200) || (response.code >= 300)) {
        LOG_S(ERROR) << "Unable to do conditonal upload of (" << source.native()
                     << ") to (" << dest.whole()
                     << "). Got error code: " << response.code
                     << ". Body: " << response.body << std::endl;
        throw UploadError() << http_code(response.code) << http_body(response.body);
      }
      #ifndef NDEBUG
      for (const auto& header: response.headers) {
        DLOG_S(5) << "- " << header.second << std::endl;
      }
      #endif
      std::string remoteMD5(response.headers.at("ETag"));
      if (md5 != remoteMD5) {
        std::ifstream file(source.native());
        conn.put(dest.path_part()).add_header("ETag", md5).body(file).go();
      }
    } catch (std::exception& e) {
      using namespace std;
      LOG_S(ERROR) << "std::exception trying to conditionally upload "
                   << source.native() << " to " << dest.whole() << ": "
                   << e.what() << std::endl;
    } catch (boost::exception& e) {
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
