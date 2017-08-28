/// Logs in to rackspace and gives you a token
#pragma once

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <map>
#include <string>

#include "logging.hpp"

namespace cdnalizerd {

/**
* @brief Thrown when we can't log in to rackspace
*/
struct LoginFailed : std::runtime_error {
  LoginFailed(std::string msg) : std::runtime_error(msg) {}
};

using namespace std::string_literals;
namespace pt = boost::property_tree;

class Rackspace {
private:
  /// Our access token
  std::string _token;
  /// The response from the login endpoint
  pt::ptree data;
  /// Maps cloud files region to URLs
  std::map<std::string, std::string> cloudFilesPublicURLs;
  std::map<std::string, std::string> cloudFilesPrivateURLs;

public:
  Rackspace(pt::ptree &&data) : data(data) {
    _token = data.get<std::string>("access.token.id");
    const pt::ptree* cloudFiles(nullptr);
    for (const auto& pair : data.get_child("access.serviceCatalog")) {
      if (pair.second.get<std::string>("name") == "cloudFiles") {
        cloudFiles = &pair.second;
        break;
      }
    }
    if (!cloudFiles) {
      std::stringstream tmp;
      pt::write_json(tmp, data);
      LOG_S(FATAL)
          << "Couldn't find 'cloudFiles' in serviceCatalog in this json: "
          << tmp.str();
    }
    const pt::ptree &endpoints(cloudFiles->get_child("endpoints"));
    for (const auto &pair : endpoints) {
      const pt::ptree ep(pair.second);
      cloudFilesPublicURLs[ep.get<std::string>("region")] =
          ep.get<std::string>("publicURL");
      cloudFilesPrivateURLs[ep.get<std::string>("region")] =
          ep.get<std::string>("internalURL");
    }
  }
  const pt::ptree &loginJSON() const { return data; }
  const std::string &token() const { return _token; }
  /// Returns the cloud files url for your region
  const std::string &getURL(const std::string &region, bool snet) const {
    if (snet)
      return cloudFilesPrivateURLs.at(region);
    else
      return cloudFilesPublicURLs.at(region);
  }
};
}
