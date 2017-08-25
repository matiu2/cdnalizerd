#include <iostream>
#include <sstream>
#include <stdexcept>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/beast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#define LOGURU_IMPLEMENTATION 1
#include <loguru.hpp>

#include <cstdlib>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;
namespace pt = boost::property_tree;

std::pair<std::string, std::string> getCredentials() {
  // Get user's home directory
  char *username = std::getenv("OS_USERNAME");
  char *password = std::getenv("OS_PASSWORD");
  if ((username == nullptr) || (password == nullptr))
    throw std::runtime_error("Unable to login, please add the environment "
                             "varables. "
                             "eg.\nOS_USERNAME=abc\nOS_PASSWORD=5320203405430");
  return std::make_pair(username, password);
}

int testLogin(boost::asio::yield_context yield, asio::io_service& ios) {
  std::stringstream output;
  auto creds = getCredentials();
  int result = 0;
  tcp::resolver dns{ios};
  tcp::socket sock{ios};

  // ssl context
  ssl::context ctx{ssl::context::tlsv12};
  ctx.set_default_verify_paths();

  // ssl stream
  const std::string host("identity.api.rackspacecloud.com");
  ssl::stream<asio::ip::tcp::socket &> s(sock, ctx);
  s.set_verify_mode(ssl::verify_peer);
  s.set_verify_callback(ssl::rfc2818_verification(host));
  auto const lookup = dns.async_resolve({host, "https"}, yield);
  asio::async_connect(sock, lookup, yield);
  sock.set_option(tcp::no_delay(true));
  s.handshake(decltype(s)::client);
  
  // Make the request body
  boost::property_tree::ptree out;
  out.put("auth.RAX-KSKEY:apiKeyCredentials.username", creds.first);
  out.put("auth.RAX-KSKEY:apiKeyCredentials.apiKey", creds.second);
  std::stringstream tmp; 
  pt::write_json(tmp, out);
   
  // Make the request
  http::request<http::string_body> req(
      http::verb::post, "/v2.0/tokens", 11);
  req.set(http::field::host, host);
  req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
  req.set(http::field::content_type, "application/json");
  req.set(http::field::accept, "application/json");
  req.set(http::field::content_length, tmp.str().size());
  req.body = tmp.str();

  LOG_S(1) << "Sending Request: " << req;
  try {
    http::async_write(s, req, yield);
  } catch (boost::system::system_error& e) {
    LOG_S(FATAL) << e.code().message();
    return -1;
  } catch (std::exception& e) {
    LOG_S(FATAL) << e.what();
    return -1;
  } catch (...) {
    LOG_S(FATAL) << "Unknown error";
    return -1;
  }
  
  // This buffer is used for reading and must be persisted
  boost::beast::flat_buffer buffer;

  // Declare a container to hold the response
  http::response<http::string_body> res;

  // Receive the HTTP response
  http::async_read(s, buffer, res, yield);
  if (res.result() != http::status::ok) {
    LOG_S(ERROR) << "Invalid response code: " << res.result() << " body:\n" << res;
    throw std::runtime_error("Bad http response code");
  }

  boost::system::error_code ec;
  s.async_shutdown(yield[ec]);

  pt::ptree data;
  std::stringstream in(res.body.data()); 
  pt::read_json(in, data);
  std::string token = data.get<std::string>("access.token.id");
  LOG_S(INFO) << "Got token: " << token;

  using asio::error::misc_errors;
  using asio::error::basic_errors;
  const auto &misc_cat = asio::error::get_misc_category();
  const auto &ssl_cat = asio::error::get_ssl_category();
  // This error means the remote party has initiated has already closed the
  // underlying transport (TCP FIN) without shutting down the SSL.
  // It may be a truncate attack attempt, but nothing we can do about it
  // except close the connection.
  if (ec.category() == ssl_cat &&
      ec.value() == ERR_PACK(ERR_LIB_SSL, 0, SSL_R_SHORT_READ)) {
    // SSL Shutdown - remote party just dropped TCP FIN instead of closing
    // SSL protocol. Possible truncate attack - closing connection.
    return result;
  }
  // We are the first one to run ssl_shutdown, and remote party responded in
  // kind, just continue
  if (ec.category() == misc_cat && ec.value() == misc_errors::eof) {
    return result;
  }
  // The remote party sent ssl_shutdown, then just dropped the connection
  if (ec.category() == misc_cat &&
      ec.value() == basic_errors::operation_aborted) {
    return result;
  }
  // Everything went as planned
  if (ec.category() == boost::system::system_category() &&
      ec.value() == boost::system::errc::success) {
    return result;
  }

  // Something scary happened, log an error (throw an exception too)
  LOG_S(FATAL) << "Unable to close down SSL connection";

  return result;
}

int main(int argc, char *argv[]) {
  int result;
  loguru::g_stderr_verbosity = 9;
  boost::asio::io_service ios;
  boost::asio::spawn(ios, [&ios] (asio::yield_context y) { testLogin(y, ios); });
  ios.run();
  return result;
}
