/// Logs in to rackspace and gives you a token
#pragma once

#include <parse_to_json_class.hpp>
#include <curlpp11.hpp>

#include <stdexcept>

namespace cdnalizerd {

/**
* @brief Thrown when we can't log in to rackspace
*/
struct LoginFailed : std::runtime_error {
  LoginFailed(std::string msg) : std::runtime_error(msg) {}
};

struct Rackspace {
  /// Our access token
  std::string token;
  /// The response from the login endpoint
  json::JSON response;
  /// Returns true if we are logged in and have an access token
  operator bool() const { return !token.empty(); }
  void login(const std::string &username, const std::string &apikey) {
    using namespace json;
    // Create the request
    JSON j(JMap{
        {"auth", JMap{{"RAX-KSKEY:apiKeyCredentials",
                       JMap{{"username", username}, {"apiKey", apikey}}}}}});
    std::stringstream req_body;
    req_body << j;
    // Get the response
    std::string response_string;
    curl::Easy c;
    c.url("https://identity.api.rackspacecloud.com/v2.0/tokens")
        .header("Content-type: application/json")
        .userAgent("cdnalizerd 0.1")
        .POST()
        .customBody(req_body)
        .perform(response_string);
    response = readValue(response_string.begin(), response_string.end());
    // Make sure we got the token
    try {
      token = response.at("access").at("token").at("id");
    } catch (std::out_of_range) {
      throw LoginFailed(std::string("Couldn't get access token. Response: ") +
                        response_string);
    }
    if (token.empty())
      throw LoginFailed(std::string("Empty access token. Response: ") +
                        response_string);
  }
};
}
