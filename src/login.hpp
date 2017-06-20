/// Logs in to rackspace and gives you a token
#pragma once

#include <jsonpp11/json_class.hpp>
#include <jsonpp11/parse_to_json_class.hpp>
#include <RESTClient/rest.hpp>

#include <stdexcept>

namespace cdnalizerd {

/**
* @brief Thrown when we can't log in to rackspace
*/
struct LoginFailed : std::runtime_error {
  LoginFailed(std::string msg) : std::runtime_error(msg) {}
};

using namespace std::string_literals;

using RESTClient::http::yield_context;
using RESTClient::http::URL;
using RESTClient::http::Response;
using RESTClient::REST;

class Rackspace {
private:
  yield_context& yield;
  /// Our access token
  std::string _token;
  /// The response from the login endpoint
  json::JSON response;
public:
  Rackspace(yield_context& yield) : yield(yield) {}
  /// Returns true if we are logged in and have an access token
  operator bool() const { return !_token.empty(); }
  void login(const std::string &username, const std::string &apikey) {
    using namespace json;
    // Create the request
    JSON j(JMap{
        {"auth", JMap{{"RAX-KSKEY:apiKeyCredentials",
                       JMap{{"username", username}, {"apiKey", apikey}}}}}});
    std::stringstream req_body;
    req_body << j;
    // Get the response
    REST api(yield, "https://identity.api.rackspacecloud.com"s,
             {{"Content-type", "application/json"},
              {"User-Agent", "cdnalizerd 0.2"}});
    Response httpResponse = api.post("/v2.0/tokens").body(req_body).go();
    if (httpResponse.ok != "OK")
      throw LoginFailed("Got bad response from API: " +
                        std::to_string(httpResponse.code) + " " +
                        httpResponse.ok + " " + httpResponse.body);
    response = readValue(httpResponse.body.begin(), httpResponse.body.end());
    // Make sure we got the token
    try {
      _token = response.at("access").at("token").at("id");
    } catch (std::out_of_range) {
      throw LoginFailed(std::string("Couldn't get access token. Response: ") +
                        httpResponse.body);
    }
    if (_token.empty())
      throw LoginFailed(std::string("Empty access token. Response: ") +
                        response.toString());
  }
  const json::JSON& loginJSON() const { return response; }
  const std::string& token() const { return _token; }
};
}
