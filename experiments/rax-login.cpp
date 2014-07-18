/// Experiment that logs in to Rackspace US API Servers

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

struct CURLHeaders {
  curl_slist* base=nullptr;
  ~CURLHeaders() {
    if (base)
      curl_slist_free_all(base);
  }
  void add(const char* header) {
    curl_slist* last = curl_slist_append(base, header);
    if (!base)
      base = last;
  }
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

class CurlEasy {
protected:
  CURLHeaders headers;
  CURL* handle;
public:
  CurlEasy() : handle(curl_easy_init()) {
    // Show error messages on failure
    setOpt(CURLOPT_VERBOSE, 0);
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
  void addHeader(const char* header) {
    headers.add(header);
    setOpt(CURLOPT_HTTPHEADER, headers.base);
  }
};

/// Called by curl when we have data to recv
extern "C"
size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  // @userdata is a curl object
  const size_t bytes = size * nmemb;
  std::string* response = static_cast<std::string*>(userdata);
  response->reserve(response->size() + bytes);
  std::copy(ptr, ptr+bytes, std::back_inserter(*response));
  return bytes;
}

/// Called by curl when it wants data to send
extern "C"
size_t read_callback(char *buffer, size_t size, size_t nitems, void *instream) {
  const size_t bytes = size * nitems;
  auto data = static_cast<std::stringstream *>(instream);
  data->read(buffer, bytes);
  return data->gcount();
}

int main(int argc, const char *argv[]) {
  using namespace json;
  CurlGlobalSentry curl;

  // Get the username and api key from the command line
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " username apiKey" << std::endl;
    return -1;
  }
  std::string username(argv[1]);
  std::string apiKey(argv[2]);

  // The body is json
  JSON j(
      JMap{{"auth", JMap{{"RAX-KSKEY:apiKeyCredentials",
                          JMap{{"username", username}, {"apiKey", apiKey}}}}}});

  std::stringstream req_body;
  req_body << j;
  std::cout << req_body.str() << std::endl;

  // Send it
  CurlEasy c;
  std::string response_string;
  c.setOpt(CURLOPT_URL, "https://identity.api.rackspacecloud.com/v2.0/tokens");
  //c.setOpt(CURLOPT_URL, "http://posttestserver.com/post.php");
  //c.setOpt(CURLOPT_URL, "https://posttestserver.com/post.php");
  c.setOpt(CURLOPT_POST, 1);
  c.addHeader("Content-type: application/json");
  c.setOpt(CURLOPT_USERAGENT , "cdnalizerd 0.1");
  c.setOpt(CURLOPT_READFUNCTION, read_callback);
  c.setOpt(CURLOPT_POSTFIELDSIZE, req_body.str().size());
  c.setOpt(CURLOPT_READDATA, &req_body);
  c.setOpt(CURLOPT_WRITEFUNCTION, write_callback);
  c.setOpt(CURLOPT_WRITEDATA, &response_string);
  c.perform();
  JSON response = json::read(response_string);
  std::cout << "Token: " << response["access"]["token"]["id"] << std::endl;
}
