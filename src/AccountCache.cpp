#include "AccountCache.hpp"

#include <algorithm>
#include <list>

#include "https.hpp"

namespace cdnalizerd {

void fillAccountCache(yield_context &yield, const Config &config,
                      AccountCache &cache, std::function<void()> onDone) {
  // Make the request outline
  const std::string host("identity.api.rackspacecloud.com");
  http::request<http::string_body> req(http::verb::post, "/v2.0/tokens", 11);
  req.set(http::field::host, host);
  req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
  req.set(http::field::content_type, "application/json");
  req.set(http::field::accept, "application/json");
  
  // This buffer is used for reading and must be persisted
  beast::flat_buffer buffer;

  // Connect
  HTTPS conn(yield, host);
  for (const ConfigEntry &configEntry : config.entries()) {
    // Write the json for the request body
    nlohmann::json json{{"auth",
                         {{"RAX-KSKEY:apiKeyCredentials",
                           {{"username", *configEntry.username},
                            {"apiKey", *configEntry.apikey}}}}}};
    req.body = json.dump();
    req.set(http::field::content_length, req.body.size());
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
    Rackspace rs(json::parse(res.body));
    cache.emplace(configEntry.username, std::move(rs));

    onDone();
  }
}

}
