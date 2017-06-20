/// Experiment that logs in to Rackspace US API Servers
/// **** This is no longer used, since we changed away from curlpp11 to RESTClient2 library ****

#include <iostream>
#include <stdexcept>
#include <iterator>
#include <algorithm>

#include <parse_to_json_class.hpp>

using namespace json;

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
  JSON j(
      JMap{{"auth", JMap{{"RAX-KSKEY:apiKeyCredentials",
                          JMap{{"username", username}, {"apiKey", apiKey}}}}}});

  std::stringstream req_body;
  req_body << j;
  std::string response_string;

  // Send it
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
