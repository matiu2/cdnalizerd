#pragma once

#include "login.hpp"
#include "../AccountCache.hpp"
#include "../config_reader/config.hpp"
#include "../logging.hpp"

#include <RESTClient/rest.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/coroutine2/coroutine.hpp>

#include <iterator>
#include <string>

namespace cdnalizerd {

using namespace std::literals::string_literals;

struct ContainerEntry {
  const std::string& username;
  const std::string& container;
  std::vector<std::string> files;
};

using ListContainerCoroutine = boost::coroutines::coroutine<std::vector<std::string>>;
using ListContainerResult = ListContainerCoroutine::pull_type;
using ListContainerOut = ListContainerCoroutine::push_type;

/// Fills 'out' with the contents of a container. Returns true if we need to
void doListContainer(ListContainerOut &out, yield_context &yield,
                     const AccountCache &accounts, const ConfigEntry &entry) {
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

    BOOST_LOG_TRIVIAL(debug) << "Requesting " << baseURL.host_part() + path;
    auto response = conn.get(path).go();
    auto begin =
        boost::make_split_iterator(response.body, boost::first_finder("\n"));
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
    data = {};
    // If we didn't get 'limit' results, we're done
    assert(count <= limit);
    if (count != limit)
      break;
  }
}

namespace processes {
  
ListContainerResult listContainer(yield_context &yield,
                                  const AccountCache &accounts,
                                  const ConfigEntry &entry) {
  return ListContainerResult(
      [&](ListContainerOut& pusher) { doListContainer(pusher, yield, accounts, entry); });
}

/// Prints out the contents of all containers 
void listContainers(yield_context yield, const Config& config) {
  AccountCache accounts;
  login(yield, accounts, config);
  using namespace std;
  for (const ConfigEntry& entry : config.entries()) {
    cout << "========\n"
         << "Username: " << *entry.username << '\n'
         << "Container: " << *entry.container << "\n\n";
    for (std::vector<std::string> files :
         listContainer(yield, accounts, entry))
      for (const std::string& filename : files)
        cout << filename << '\n';
  }
}

} /* processes { */ 
} /* cdnalizerd  */
