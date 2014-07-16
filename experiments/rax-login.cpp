/// Experiment that logs in to Rackspace US API Servers

//#include "curl_easy.h"

#include <iostream>
#include <stdexcept>
#include <iterator>
#include <algorithm>

#include <json.hpp>
#include <curl.h>

struct CurlErr : std::runtime_error {
  CURLcode code;
  CurlErr(CURLcode code) : std::runtime_error(curl_easy_strerror(code)), code(code) {}
};

class CurlEasy {
protected:
  CURL* handle;
public:
  CurlEasy() : handle(curl_easy_init()) {
    // Show error messages on failure
    setOpt(CURLOPT_VERBOSE, 1);
  }
  ~CurlEasy() { curl_easy_cleanup(handle); }
  template <typename ...T> void setOpt(CURLoption opt, T... values) const {
    checkError(curl_easy_setopt(handle, opt, values...));
  }
  template <typename... Types> void getInfo(CURLINFO info, Types... args) const {
    checkError(curl_easy_getinfo(handle, info, args...));
  }
  void perform() const { checkError(curl_easy_perform(handle)); }
  void checkError(CURLcode num) const {
    if (num != CURLE_OK)
      throw CurlErr(num);
  }
};

/// Called by curl when we have data to recv
size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  // @userdata is a curl object
  const size_t bytes = size * nmemb;
  std::string* response = static_cast<std::string*>(userdata);
  response->reserve(response->size() + bytes);
  std::copy(ptr, ptr+bytes, std::back_inserter(*response));
  return bytes;
}

/// Called by curl when it wants data to send
size_t read_callback(char *buffer, size_t size, size_t nitems, void *instream) {
  const size_t bytes = size * nitems;
  auto data = static_cast<std::stringstream *>(instream);
  data->read(buffer, bytes);
  return bytes;
}

int main(int argc, const char *argv[]) {
  using namespace json;

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

  std::stringstream req_body;
  req_body << j;

  // Send it
  CurlEasy c;
  std::string response;
  c.setOpt(CURLOPT_URL, "https://identity.api.rackspacecloud.com/v2.0/tokens");
  c.setOpt(CURLOPT_POST, 1);
  c.setOpt(CURLOPT_HTTPHEADER, "Content-type: application/json");
  c.setOpt(CURLOPT_USERAGENT , "cdnalizerd 0.1");
  c.setOpt(CURLOPT_ERRORBUFFER, 1);
  c.setOpt(CURLOPT_READDATA, &req_body);
  c.setOpt(CURLOPT_READFUNCTION, write_callback);
  c.setOpt(CURLOPT_WRITEDATA, &response);
  c.setOpt(CURLOPT_WRITEFUNCTION, read_callback);
  c.perform();
  std::cout << "Received: " << response << std::endl;
}
