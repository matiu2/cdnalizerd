#include "list.hpp"

#include <RESTClient/rest.hpp>
#include <boost/algorithm/string/split.hpp>
#include <jsonpp11/parse_to_json_class.hpp>

namespace cdnalizerd {

using namespace std::literals;

void genericDoListContainer(
    std::function<size_t(const std::string &, std::string &)> out,
    yield_context &yield, const Rackspace &rackspace, const ConfigEntry &entry,
    bool restrict_to_remote_dir, std::string extra_params = "") {
  RESTClient::http::URL baseURL(rackspace.getURL(*entry.region, entry.snet));
  std::clog << "INFO: Connecting to " << baseURL.host_part() << std::endl;
  RESTClient::REST conn(yield, baseURL.host_part(),
                        {{"Content-type", "application/json"},
                         {"X-Auth-Token", rackspace.token()}});
  const size_t limit = 10000;
  std::string marker;
  std::string prefix;
  if (restrict_to_remote_dir && (!entry.remote_dir.empty()))
    prefix = entry.remote_dir;
  while (true) {
    std::string path(baseURL.path_part() + "/" + *entry.container + "?limit=" +
                     std::to_string(limit));
    if (!marker.empty())
      path += "&marker=" + marker;
    if (!prefix.empty())
      path += "&prefix=" + prefix;
    if (!extra_params.empty())
      path.append(extra_params);
    std::clog << "DEBUG: Requesting " << baseURL.host_part() + path
              << std::endl;
    auto response = conn.get(path).go();
    std::clog << "DEBUG: Downloading info on "
              << response.headers["X-container-Object-Count"] << " objects"
              << std::endl;
    size_t count = out(response.body, marker);
    // If we didn't get 'limit' results, we're done
    assert(count <= limit);
    if (count != limit)
      break;
  }
}

/// Fills 'out' with the contents of a container. Returns true if we need to
void doListContainer(ListContainerOut &out, yield_context &yield,
                     const Rackspace &rackspace, const ConfigEntry &entry) {
  auto handleOutput = [&out](const std::string &body, std::string &marker) {
    auto begin = boost::make_split_iterator(body, boost::first_finder("\n"));
    decltype(begin) end;
    size_t count = 0;
    std::vector<std::string> data;
    while (begin != end) {
      std::string entry(begin->begin(), begin->end());
      if (!entry.empty()) {
        data.emplace_back(std::move(entry));
        ++count;
      }
      ++begin;
    }
    if (!data.empty())
      marker = data.back();
    // Yield our results
    out(std::move(data));
    return count;
  };
  genericDoListContainer(handleOutput, yield, rackspace, entry, false);
}

void JSONDoListContainer(JSONListContainerOut &out, yield_context &yield,
                         const Rackspace &rackspace, const ConfigEntry &entry,
                         bool restrict_to_remote_dir) {
  genericDoListContainer(
      [&out](const std::string &body, std::string &marker) {
        json::JList entries = json::readValue(body);
        size_t count = entries.size();
        if (count == 0)
          return count;
        const json::JMap &last = entries.back();
        marker = last.at("name");
        out(std::move(entries));
        return count;
      },
      yield, rackspace, entry, restrict_to_remote_dir, "&format=json");
}

namespace processes {

/// Prints out the contents of all containers
void listContainers(yield_context yield, const AccountCache &accounts,
                    const Config &config) {
  using namespace std;
  for (const ConfigEntry &entry : config.entries()) {
    cout << "========\n"
         << "Username: " << *entry.username << '\n'
         << "Container: " << *entry.container << "\n\n";
    const Rackspace &rs(accounts.at(entry.username));
    for (std::vector<std::string> files : listContainer(yield, rs, entry)) {
      for (const std::string &filename : files)
        cout << filename << '\n';
    }
  }
}

/// Prints out the contents of all containers
void JSONListContainers(yield_context yield, const AccountCache &accounts,
                        const Config &config) {
  using namespace std;
  for (const ConfigEntry &entry : config.entries()) {
    cout << "========\n"
         << "Username: " << *entry.username << '\n'
         << "Container: " << *entry.container << "\n\n";
    for (json::JList files :
         JSONListContainer(yield, accounts.at(entry.username), entry, false))
      for (const json::JMap &data : files)
        cout << data.at("name") << " - " << data.at("hash") << " - "
             << data.at("last_modified") << " - " << data.at("content_type")
             << " - " << data.at("bytes") << '\n';
  }
}

} /* processes  */

} /* cdnalizerd  */
