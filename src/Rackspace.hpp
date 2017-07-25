/// Logs in to rackspace and gives you a token
#pragma once

#include "logging.hpp"

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
public:
  enum Status {
    Fresh,     // Doesn't have a token yet
    LoggingIn, // Is in the process of logging in
    Error,     // Login failed
    Ready      // Logged in ard ready to be used
  };
private:
  /// Our access token
  std::string _token;
  /// The response from the login endpoint
  json::JSON response;
  Status _status;
  /// Maps cloud files region to URLs
  std::map<std::string, std::string> cloudFilesPublicURLs;
  std::map<std::string, std::string> cloudFilesPrivateURLs;
public:
  Rackspace() : _status(Fresh) {}
  /// Makes a new API for use by login
  REST makeAPI(yield_context &yield,
               std::string url = "https://identity.api.rackspacecloud.com"s) {
    return REST(yield, url, {{"Content-type", "application/json"},
                             {"User-Agent", "cdnalizerd 0.2"}});
  }
  /// Returns true if we are logged in and have an access token
  void login(const std::string &username, const std::string &apikey, REST& api) {
    struct Sentry {
      Status& s;
      Sentry(Status& s) : s(s) {
        s = LoggingIn;
      }
      ~Sentry() {
        if (s == LoggingIn)
          s = Error;
      }
    };
    Sentry s(_status);
    using namespace json;
    // Create the request
    JSON j(JMap{
        {"auth", JMap{{"RAX-KSKEY:apiKeyCredentials",
                       JMap{{"username", username}, {"apiKey", apikey}}}}}});
    std::stringstream req_body;
    req_body << j;
    // Get the response
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
    // Fill in the cloudFiles URLs
    JList& catalog(response["access"]["serviceCatalog"]);
    for (JMap& entry : catalog) {
      if (entry["name"] == "cloudFiles") {
        JList& endpoints(entry["endpoints"]);
        for (const JMap& ep : endpoints) {
          cloudFilesPublicURLs[ep.at("region")] = ep.at("publicURL");
          cloudFilesPrivateURLs[ep.at("region")] = ep.at("internalURL");
        }
      }
    }
    _status = Ready;
  }
  const json::JSON& loginJSON() const { return response; }
  const std::string& token() const { return _token; }
  Status status() const { return _status; }
  /// Returns the cloud files url for your region
  const std::string& getURL(const std::string& region, bool snet) {
    assert(status() == Ready);
    if (snet)
      return cloudFilesPrivateURLs[region];
    else
      return cloudFilesPublicURLs[region];
  }

};
}
