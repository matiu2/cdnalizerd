#include "list.hpp"

#include "../logging.hpp"
#include "login.hpp"

#include <RESTClient/rest.hpp>
#include <boost/algorithm/string/split.hpp>
#include <jsonpp11/parse_to_json_class.hpp>

namespace cdnalizerd {

void genericDoListContainer(std::function<size_t(const std::string &, std::string&)> out,
                            yield_context &yield, const AccountCache &accounts,
                            const ConfigEntry &entry, std::string extra_params="") {
  auto rackspace = accounts.at(entry.username);
  RESTClient::http::URL baseURL(rackspace.getURL(*entry.region, entry.snet));
  BOOST_LOG_TRIVIAL(info) << "Connecting to " << baseURL.host_part();
  RESTClient::REST conn(yield, baseURL.host_part(),
                        {{"Content-type", "application/json"},
                         {"X-Auth-Token", rackspace.token()}});
  std::string marker; // Where to start
  const size_t limit = 10000;
  while (true) {
    std::string path(baseURL.path_part() + "/" + *entry.container + "?limit=" +
                     std::to_string(limit));
    if (!marker.empty())
      path += "&marker=" + marker;
    if (!extra_params.empty())
      path.append(extra_params);

    BOOST_LOG_TRIVIAL(debug) << "Requesting " << baseURL.host_part() + path;
    auto response = conn.get(path).go();
    BOOST_LOG_TRIVIAL(debug) << "Downloading info on "
                             << response.headers["X-container-Object-Count"]
                             << " objects";
    size_t count = out(response.body, marker);
    // If we didn't get 'limit' results, we're done
    assert(count <= limit);
    if (count != limit)
      break;
  }
}

/// Fills 'out' with the contents of a container. Returns true if we need to
void doListContainer(ListContainerOut &out, yield_context &yield,
                     const AccountCache &accounts, const ConfigEntry &entry) {
  genericDoListContainer(
      [&out](const std::string &body, std::string &marker) {
        auto begin =
            boost::make_split_iterator(body, boost::first_finder("\n"));
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
      },
      yield, accounts, entry);
}

void JSONDoListContainer(JSONListContainerOut &out, yield_context &yield,
                         const AccountCache &accounts,
                         const ConfigEntry &entry) {
  genericDoListContainer(
      [&out](const std::string &body, std::string& marker) {
        BOOST_LOG_TRIVIAL(debug) << "About to parse json: \n" << body;
        json::JList entries = json::readValue(body);
        size_t count = entries.size();
        const json::JMap& last = entries.back();
        marker = last.at("name");
        out(std::move(entries));
        return count;
      },
      yield, accounts, entry, "&format=json");
}

namespace processes {

/// Prints out the contents of all containers
void listContainers(yield_context yield, const Config &config) {
  AccountCache accounts;
  login(yield, accounts, config);
  using namespace std;
  for (const ConfigEntry &entry : config.entries()) {
    cout << "========\n"
         << "Username: " << *entry.username << '\n'
         << "Container: " << *entry.container << "\n\n";
    for (std::vector<std::string> files : listContainer(yield, accounts, entry))
      for (const std::string &filename : files)
        cout << filename << '\n';
  }
}

/// Prints out the contents of all containers
void JSONListContainers(yield_context yield, const Config &config) {
  AccountCache accounts;
  login(yield, accounts, config);
  using namespace std;
  for (const ConfigEntry &entry : config.entries()) {
    cout << "========\n"
         << "Username: " << *entry.username << '\n'
         << "Container: " << *entry.container << "\n\n";
    for (json::JList files : JSONListContainer(yield, accounts, entry))
      for (const json::JMap &data : files)
        cout << data.at("name") << " - " << data.at("hash") << " - "
             << data.at("last_modified") << " - " << data.at("content_type")
             << " - " << data.at("bytes") << '\n';
  }
}


} /* processes  */

} /* cdnalizerd  */
