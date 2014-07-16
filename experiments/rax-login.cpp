/// Experiment that logs in to Rackspace US API Servers
/// ABANDONED: I'm dropping libcurl in favor of cpp-netlib

#include <iostream>
#include <stdexcept>
#include <system_error>
#include <sstream>

#include <json.hpp>
#include <sys/select.h>
#include <boost/network/protocol/http/client.hpp>

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
  JSON j(JMap{
      {"auth", JMap{{"RAX-KSKEY:apiKeyCredentials",
                     JMap{{"username", username}, {"apiKey", apiKey}}}}}});

  std::stringstream request_body;
  request_body << j;
  std::cout << "Request Body: " << std::endl << request_body.str() << std::endl;

  // Send it
  namespace http = boost::network::http;
  namespace net = boost::network;

  http::client client;
  http::client::request req("https://identity.api.rackspacecloud.com/v2.0/tokens");
  req << net::header("Content-type", "application/json");
  auto response = client.post(req, request_body.str());
  std::string response_body = response.body();
  //net::headers_wrapper::container_type const& headers_ = headers(response_);
  std::cout << "Reply: " << response_body << std::endl;
  JSON data = json::read(response_body);
  std::cout << "Token: " << data["access"]["token"]["id"] << std::endl;
}
