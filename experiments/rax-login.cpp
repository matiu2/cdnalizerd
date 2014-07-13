/// Experiment that logs in to Rackspace US API Servers
/// ABANDONED: I'm dropping libcurl in favor of cpp-netlib

//#include "curl_easy.h"

#include <iostream>
#include <stdexcept>
#include <system_error>
#include <sstream>

#include <json.hpp>
#include <curl.h>
#include <sys/select.h>

struct CurlErr : std::runtime_error {
  CURLcode code;
  CurlErr(CURLcode code) : std::runtime_error(curl_easy_strerror(code)), code(code) {}
};

/// A create once per app, in the main curl sentry
struct CurlGlobalSentry {
  CurlGlobalSentry() {
    CURLcode result = curl_global_init(CURL_GLOBAL_SSL);
    if (result != 0) {
      std::stringstream msg;
      msg << "Couldn't Initialize Curl Library. Error code: " << result;
      throw std::runtime_error(std::move(msg.str()));
    }
  }
  ~CurlGlobalSentry() {
    curl_global_cleanup();
  }
};

namespace {
thread_local std::function<size_t(char* buffer, size_t size, size_t nitems, void* instream)> callback;
extern "C" size_t read_callback(char *buffer, size_t size, size_t nitems, void *instream) {
  return callback

}
}

/// A single curl handle. One per thread please, but can be re-used for multiple requests.
class CurlEasy {
protected:
  CURL* handle;
public:
  CurlEasy() : handle(curl_easy_init()) {
    // Show error messages on failure
    setOpt(CURLOPT_VERBOSE, 1);
    setOpt(CURLOPT_SSLENGINE_DEFAULT, 1);
  }
  ~CurlEasy() { curl_easy_cleanup(handle); }
  template <typename T> void setOpt(CURLoption opt, T value) const {
    checkError(curl_easy_setopt(handle, opt, value));
  }
  template <typename... Types> void getInfo(CURLINFO info, Types... args) const {
    checkError(curl_easy_getinfo(handle, info, args...));
  }
  void perform() const { checkError(curl_easy_perform(handle)); }
  void checkError(CURLcode num) const {
    if (num != CURLE_OK)
      throw CurlErr(num);
  }
  void getSocket(fd_set& result) const {
    int socket;
    checkError(curl_easy_getinfo(handle, CURLINFO_LASTSOCKET, &socket));
    FD_ZERO(&result);
    FD_SET(socket, &result);
  }
  void ensureWritable() {
    /// Ensures we can send on the connection
    fd_set writable;
    getSocket(writable);
    struct timeval tv{1, 0}; // Wait 1 second and 0 microsends
    int res = select(1, nullptr, &writable, nullptr, &tv);
    if (res < 0)
      throw std::system_error(std::error_code(static_cast<int>(errno), std::system_category()));
  }
  void ensureReadable() {
    fd_set writable;
    getSocket(writable);
    struct timeval tv{1, 0}; // Wait 1 second and 0 microsends
    int res = select(1, nullptr, &writable, nullptr, &tv);
    if (res < 0)
      throw std::system_error(std::error_code(static_cast<int>(errno), std::system_category()));
  }
  void send(const std::string& data) {
    ensureWritable();
    size_t sent=0;
    checkError(curl_easy_send(handle, data.c_str(), data.size(), &sent));
    assert(sent == data.size()); // Didn't send all the data. Something's fishy.
  }
  std::string recv() {
    ensureReadable();
    size_t recieved=0;
    checkError(curl_easy_send(handle, data.c_str(), data.size(), &received));
    assert(received == data.size()); // Didn't send all the data. Something's fishy.


  }
};

int main(int argc, const char *argv[]) {
  using namespace json;
  CurlGlobalSentry curlSentry;

  // Get the username and api key from the command line
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " username apiKey" << std::endl;
    return -1;
  }
  std::string username(argv[1]);
  std::string apiKey(argv[2]);

  // The body is json
  JSON j(JList{JMap{
      {"auth", JMap{{"RAX-KSKEY:apiKeyCredentials",
                     JMap{{"username", username}, {"apiKey", apiKey}}}}}}});

  std::stringstream body;
  body << j;

  // Send it
  CurlEasy c;
  c.setOpt(CURLOPT_URL, "https://identity.api.rackspacecloud.com/v2.0/tokens");
  c.setOpt(CURLOPT_POST, 1);
  c.setOpt(CURLOPT_CAPATH, "/etc/ssl/certs"); // TODO: change for non-ubuntu releases
  c.setOpt(CURLOPT_CONNECT_ONLY, 1);
  c.setOpt(CURLOPT_FOLLOWLOCATION, 1);
  c.perform();
  c.send("Content-type: application/json\n\n"); // Sending our own headers. yay.
  c.send(body.str());
  std::string response(8192);

}
