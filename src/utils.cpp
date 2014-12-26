#include "utils.hpp"

namespace cdnalizerd {

std::string getContainerUrl(const Rackspace &login, const ConfigEntry &config) {
  assert(login); // Can't watch a dir until we're logged in to rackspace
  const json::JList &regions =
      login.response.at("access").at("serviceCatalog").at("cloudFiles").at(
          "endPoints");
  std::string url;
  // We get a list of regions, it doesn't come as a json dict, so we have to
  // scan through them to find our configured region
  for (const auto &region : regions) {
    if (region.at("region") == config.region()) {
      if (config.snet())
        url = region.at("privateURL");
      else
        url = region.at("publicURL");
      break;
    }
  }
  if (url.empty())
    throw std::runtime_error(
        std::string("Unable to find url for cloud files region: ") +
        config.region());
  auto join_to_url = [&url](const std::string &extra) {
    if (url.back() != '/')
      url.append("/");
    url.append(extra);
    if (url.back() != '/')
      url.append("/");
    return url;
  };
  join_to_url(config.container());
  join_to_url(config.remote_dir());
  return url;
}

}
