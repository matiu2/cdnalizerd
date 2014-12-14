/// Experiment that logs in to Rackspace US API Servers

#include <iostream>
#include <stdexcept>
#include <iterator>
#include <algorithm>

#include <jsonpp11/parse_to_json_class.hpp>
#include <curlpp11.hpp>

using namespace json;

int main(int argc, const char *argv[]) {
  using namespace json;
  curl::GlobalSentry curl;

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
  std::string response_string;

  // Send it
  curl::Easy c;
    c.url("https://identity.api.rackspacecloud.com/v2.0/tokens")
    .header("Content-type: application/json")
    .userAgent("cdnalizerd 0.1")
    .POST()
    .setOpt(CURLOPT_VERBOSE, false)
    .customBody(req_body)
    .perform(response_string);
  JSON response = readValue(response_string.begin(), response_string.end());
  std::cout << "Token: " << response["access"]["token"]["id"] << std::endl;
}
