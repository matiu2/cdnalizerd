#include "AccountCache.hpp"

#include <algorithm>
#include <list>

#include "https.hpp"

namespace cdnalizerd {

void fillSingleAccountCache(yield_context &yield,
                            const ConfigEntry &configEntry, AccountCache &cache,
                            HTTPS &conn, const std::string &host,
                            beast::flat_buffer &buffer) {
  // Write the json for the request body
  nlohmann::json json{{"auth",
                       {{"RAX-KSKEY:apiKeyCredentials",
                         {{"username", configEntry.username},
                          {"apiKey", configEntry.apikey}}}}}};

  http::request<http::string_body> req(http::verb::post, "/v2.0/tokens", 11);
  // Make the request outline
  req.set(http::field::host, host);
  req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
  req.set(http::field::content_type, "application/json");
  req.set(http::field::accept, "application/json");
  req.body() = json.dump();
  req.set(http::field::content_length, req.body().size());
  // Make the request
  DLOG_S(1) << "Sending Request: " << req;
  try {
    http::async_write(conn.stream(), req, yield);
  } catch (boost::system::system_error &e) {
    LOG_S(FATAL) << e.code().message();
  } catch (std::exception &e) {
    LOG_S(FATAL) << e.what();
  } catch (...) {
    LOG_S(FATAL) << "Unknown error";
  }

  // Declare a container to hold the response
  http::response<http::string_body> res;

  // Receive the HTTP response
  http::async_read(conn.stream(), buffer, res, yield);

  // Update the RS object
  Rackspace rs(json::parse(res.body()));
  cache.emplace(configEntry.username, std::move(rs));
}

void fillSingleAccountCache(yield_context &yield,
                            const ConfigEntry &configEntry,
                            AccountCache &cache) {
  const std::string host("identity.api.rackspacecloud.com");
  HTTPS conn(yield, host);
  // This buffer is used for reading and must be persisted
  beast::flat_buffer buffer;
  fillSingleAccountCache(yield, configEntry, cache, conn, host, buffer);
}

void fillAccountCache(yield_context &yield, const Config &config,
                      AccountCache &cache, std::function<void()> onDone) {

  const std::string host("identity.api.rackspacecloud.com");
  HTTPS conn(yield, host);
  // This buffer is used for reading and must be persisted
  beast::flat_buffer buffer;
  // Connect
  for (const ConfigEntry &configEntry : config.entries())
    fillSingleAccountCache(yield, configEntry, cache, conn, host, buffer);
  onDone();
}

}
