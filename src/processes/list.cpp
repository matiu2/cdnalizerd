#include "list.hpp"

#include "../logging.hpp"
#include "../url.hpp"
#include "../https.hpp"
#include "../exception_tags.hpp"
#include <nlohmann/json.hpp>

#include <boost/algorithm/string/split.hpp>
#include <boost/exception/exception.hpp>
#include <boost/exception/diagnostic_information.hpp> 
#include <boost/exception_ptr.hpp>
#include <boost/hana.hpp>

namespace cdnalizerd {

namespace processes {

using json = nlohmann::json;
using namespace std::literals;

/// A generator of either vectors of strings, or a json list
/// - because the API server paginates output
/// The server returns to us a page of container entries
template <typename Payload>
using ListContainerCoroutine =
    boost::coroutines::coroutine<Payload>;
template <typename Payload>
using ListContainerPusher = typename ListContainerCoroutine<Payload>::push_type;
template <typename Payload>
using ListContainerResult = typename ListContainerCoroutine<Payload>::pull_type;

template <typename Payload>
void doGetPages(ListContainerPusher<Payload> out, yield_context &yield,
                const std::string &token, const URL &baseURL,
                const std::string &prefix, const std::string &container,
                std::string extra_params = "") {
  LOG_S(INFO) << "Connecting to " << baseURL.scheme_host_port() << std::endl;

  HTTPS conn(yield, baseURL.host);
  http::request<http::empty_body> req{http::verb::get,
                                      baseURL.path + "/" + container, 12};
  req.set(http::field::user_agent, "cdnalizerd v1.2");
  req.set("X-Auth-Token", token);

  const size_t limit = 10001;
  std::string marker;

  while (true) {
    std::string path(baseURL.path + "/" + container +
                     "?limit=" + std::to_string(limit));
    if (!marker.empty())
      path += "&marker=" + marker;
    if (!prefix.empty())
      path += "&prefix=" + prefix;
    if (!extra_params.empty())
      path.append(extra_params);
    req.target(path);
    LOG_S(6) << "HTTP Request: " << req;
    http::async_write(conn.stream(), req, yield);
    http::response<http::string_body> response;
    http::async_read(conn.stream(), conn.read_buffer, response, conn.yield);
    DLOG_S(9) << "HTTP Response: " << response;
    size_t count(1);
    switch (response.result()) {
    case http::status::ok: {
      LOG_S(6) << "Downloaded info on " << response["X-container-Object-Count"]
               << " objects" << std::endl;
      using namespace boost::hana;
      Payload data;
      if_(type_c<Payload> == type_c<nlohmann::json>,
          [&] {
            auto begin = boost::make_split_iterator(response.body(),
                                                    boost::first_finder("\n"));
            decltype(begin) end;
            std::vector<std::string> data;
            for (auto range : boost::make_iterator_range(begin, end)) {
              std::string entry(range.begin(), range.end());
              if (!entry.empty()) {
                data.emplace_back(std::move(entry));
                ++count;
              }
            }
            if (!data.empty())
              marker = data.back();
            out(std::move(data));
          },
          [&] {
            nlohmann::json data;
            nlohmann::to_json(data, std::move(response.body()));
            out(std::move(data));
          })();
    }
    case http::status::no_content: {
      LOG_S(6) << "No files in container: "
               << response["X-container-Object-Count"] << " objects"
               << std::endl;
      break;
    }
    case http::status::not_found: {
      LOG_S(ERROR) << "Path not found on server: "
                   << baseURL.scheme_host_port() + path;
    }
    case http::status::conflict: {
      // TODO: Maybe set a timer and try againg in a minute ?
      LOG_S(ERROR) << "Server unable to comply - try again later: "
                   << baseURL.scheme_host_port() + path;
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


/// Returns an iterator over pages (vectors of strings) of the remote container
template <typename Payload>
ListContainerResult<Payload>
getPages(yield_context &yield, const std::string &token, const URL &baseURL,
         const std::string &prefix, const std::string &container,
         std::string extra_params = "") {
  return ListContainerResult<Payload>(
      [&, params = std::move(extra_params) ](auto &&out) {
        doGetPages<Payload>(std::move(out), yield, token, baseURL, prefix,
                            container, std::move(params));
      });
}

/// Does the work of getting the entries, pushes them through 'out'
template <typename Pusher>
void doGetEntries(Pusher out, yield_context &yield, const std::string &token,
                  const URL &baseURL, const std::string &prefix,
                  std::string extra_params = "");

template <typename Coroutine, typename Result = typename Coroutine::pull_type,
          typename Pusher = typename Coroutine::push_type>
Result listContainer(yield_context &yield, const Rackspace &rs,
                     const ConfigEntry &entry, bool restrict_to_remote_dir,
                     std::string extra_params = "") {
  URL baseURL(rs.getURL(entry.region, entry.snet));
  std::string prefix;
  if (restrict_to_remote_dir && (!entry.remote_dir.empty()))
    prefix = entry.remote_dir;
  return Result([&](auto &&out) {
    doGetEntries<Pusher>(std::move(out), yield, rs.token(), baseURL, prefix,
                         extra_params);
  });
}

/// Returns a generator of remote entries
ListEntriesResult listContainer(yield_context &yield, const Rackspace &rs,
                                const ConfigEntry &entry,
                                std::string extra_params) {
  return listContainer<ListEntriesCoroutine>(yield, rs, entry, true,
                                             std::move(extra_params));
}

/// Returns a generator of remote detailed entries
DetailedListEntriesResult detailedListContainer(yield_context &yield,
                                                const Rackspace &rs,
                                                const ConfigEntry &entry,
                                                std::string extra_params) {
  return listContainer<DetailedListEntriesCoroutine>(yield, rs, entry, true,
                                                     std::move(extra_params));
}

template <typename Pusher>
void doGetEntries(Pusher out, yield_context &yield, const std::string &token,
                  const URL &baseURL, const std::string &prefix,
                  std::string extra_params) {
  for (auto &&page : getPages<std::vector<std::string>>(yield, token, baseURL,
                                                        prefix, extra_params))
    for (const std::string &line : page)
      out(line);
}

void doJSONGetEntries(ListEntriesPusher out, yield_context &yield,
                      const std::string &token, const URL &baseURL,
                      const std::string &prefix, std::string extra_params) {
  if (extra_params.empty())
    extra_params = "format=json";
  else
    extra_params += "&format=json";
  for (auto &&page :
       getPages<nlohmann::json>(yield, token, baseURL, prefix, extra_params)) {
    nlohmann::json data;
    nlohmann::to_json(data, std::move(page));
    for (auto &line : page)
      out(std::move(line));
  }
}

template void doGetPages<std::vector<std::string>>(
    ListContainerPusher<std::vector<std::string>> out, yield_context &yield,
    const std::string &token, const URL &baseURL, const std::string &prefix,
    const std::string &container, std::string extra_params);
}
} /* cdnalizerd  */
