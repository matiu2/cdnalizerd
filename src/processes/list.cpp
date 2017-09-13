#include "list.hpp"

#include "../logging.hpp"
#include "../url.hpp"
#include "../https.hpp"
#include "../exception_tags.hpp"
#include <json.hpp>

#include <boost/algorithm/string/split.hpp>
#include <boost/exception/exception.hpp>
#include <boost/exception/diagnostic_information.hpp> 
#include <boost/exception_ptr.hpp>

namespace cdnalizerd {

using json = nlohmann::json;
using namespace std::literals;

void genericDoListContainer(
    std::function<size_t(const std::string &, std::string &)> out,
    yield_context &yield, const Rackspace &rackspace, const ConfigEntry &entry,
    bool restrict_to_remote_dir, std::string extra_params = "") {
  URL baseURL(rackspace.getURL(*entry.region, entry.snet));
  LOG_S(INFO) << "Connecting to " << baseURL.host_part() << std::endl;

  HTTPS conn(yield, baseURL.hostname);
  http::request<http::empty_body> req{http::verb::get, baseURL.path_part() + "/" + *entry.container , 11};
      req.set(http::field::user_agent, "cdnalizerd v0.2");
      req.set(http::field::accept, "application/json");
      req.set("X-Auth-Token", rackspace.token());

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
    req.target(path);
    LOG_S(5) << "HTTP Request: " << req;
    http::async_write(conn.stream(), req, yield);
    http::response<http::string_body> response;
    http::async_read(conn.stream(), conn.read_buffer, response, conn.yield);
    DLOG_S(9) << "HTTP Response: " << response;
    size_t count(0);
    switch (response.result()) {
      case http::status::ok: {
        LOG_S(5) << "Downloaded info on "
                 << response["X-container-Object-Count"] << " objects"
                 << std::endl;
        count = out(response.body, marker);
      }
      case http::status::no_content: {
        LOG_S(5) << "No files in container: "
                 << response["X-container-Object-Count"] << " objects"
                 << std::endl;
        break;
      } 
      case http::status::not_found: {
        LOG_S(ERROR) << "Path not found on server: "
                     << baseURL.host_part() + path;
      }
      case http::status::conflict: {
        // TODO: Maybe set a timer and try againg in a minute ?
        LOG_S(ERROR) << "Server unable to comply - try again later: "
                     << baseURL.host_part() + path;
      }
      default:
        BOOST_THROW_EXCEPTION(
            boost::enable_error_info(std::runtime_error(
                "Invalid Response when trying to list container"))
            << err::http_status(response.result()));
    };
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
        auto entries = json::parse(body);
        size_t count = entries.size();
        if (count == 0)
          return count;
        const auto &last(entries.back());
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
  try {
    using namespace std;
    for (const ConfigEntry &entry : config.entries()) {
      LOG_S(5) << "Listing Config entry: " << *entry.username << " - "
               << *entry.container;
      const Rackspace &rs(accounts.at(entry.username));
      for (std::vector<std::string> files : listContainer(yield, rs, entry)) {
        for (const std::string &filename : files)
          cout << filename << '\n';
      }
    }
  } catch (boost::exception &e) {
    LOG_S(ERROR) << "listContainers failed: "
                 << boost::diagnostic_information(e, true);
  } catch (std::exception &e) {
    LOG_S(ERROR) << "listContainers failed (std::exception): "
                 << ": " << boost::diagnostic_information(e, true) << std::endl;
  } catch (...) {
    LOG_S(ERROR) << "listContainers failed (unkown exception): "
                 << boost::current_exception_diagnostic_information(true)
                 << std::endl;
  }
}

/// Prints out the contents of all containers
void JSONListContainers(yield_context yield, const AccountCache &accounts,
                        const Config &config) {
  try {
    using namespace std;
    for (const ConfigEntry &entry : config.entries()) {
      cout << "========\n"
           << "Username: " << *entry.username << '\n'
           << "Container: " << *entry.container << "\n\n";
      for (auto files :
           JSONListContainer(yield, accounts.at(entry.username), entry, false))
        for (const auto &data : files)
          cout << data.at("name") << " - " << data.at("hash") << " - "
               << data.at("last_modified") << " - " << data.at("content_type")
               << " - " << data.at("bytes") << '\n';
    }
  } catch (boost::exception &e) {
    LOG_S(ERROR) << "JSONListContainers failed: "
                 << boost::diagnostic_information(e, true);
  } catch (std::exception &e) {
    LOG_S(ERROR) << "JSONListContainers failed (std::exception): "
                 << ": " << boost::diagnostic_information(e, true) << std::endl;
  } catch (...) {
    LOG_S(ERROR) << "JSONListContainers failed (unkown exception): "
                 << boost::current_exception_diagnostic_information(true)
                 << std::endl;
  }
}

} /* processes  */

} /* cdnalizerd  */
