/// Logs in to rackspace and gives you a token
#pragma once

#include <boost/property_tree/json_parser.hpp>
#include <map>
#include <string>
#include <nlohmann/json.hpp>

#include "logging.hpp"
#include "url.hpp"

namespace cdnalizerd {

using nlohmann::json;

using namespace std::string_literals;
namespace pt = boost::property_tree;

class Rackspace {
private:
  /// Our access token
  std::string _token;
  /// The response from the login endpoint
  json data;
  /// Maps cloud files region to URLs
  std::map<std::string, URL> cloudFilesPublicURLs;
  std::map<std::string, URL> cloudFilesPrivateURLs;

public:
  Rackspace(json &&data) : data(data) {
    _token = data["access"]["token"]["id"];
    const json* cloudFiles(nullptr);
    for (const auto &entry : data["access"]["serviceCatalog"]) {
      if (entry["name"] == "cloudFiles") {
        cloudFiles = &entry;
        break;
      }
    }
    if (!cloudFiles) {
      LOG_S(FATAL)
          << "Couldn't find 'cloudFiles' in serviceCatalog in this json: "
          << data.dump();
    }
    const json& cf(*cloudFiles);
    for (const auto &ep : cf["endpoints"]) {
      cloudFilesPublicURLs[ep["region"]] =
          static_cast<const std::string&>(ep["publicURL"]);
      cloudFilesPrivateURLs[ep["region"]] =
          static_cast<const std::string&>(ep["internalURL"]);
    }
  }
  const json &loginJSON() const { return data; }
  const std::string &token() const { return _token; }
  /// Returns the hostname for the cloud files url for your region
  const URL &getURL(const std::string &region, bool snet) const {
    if (snet)
      return cloudFilesPrivateURLs.at(region);
    else
      return cloudFilesPublicURLs.at(region);
  }
};
}
