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
#include "https.hpp"

namespace asio = boost::asio;
using tcp = boost::asio::ip::tcp;
namespace beast = boost::beast;
namespace ssl = boost::asio::ssl;
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

  // SSL context
  ssl::context ctx{ssl::context::tlsv12};
  ctx.set_default_verify_paths();

  // SSL stream
  const std::string host("identity.api.rackspacecloud.com");
  cdnalizerd::HTTPS https(yield, ios, host);
  
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
    http::async_write(https.stream(), req, yield);
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
  http::async_read(https.stream(), buffer, res, yield);
  if (res.result() != http::status::ok) {
    LOG_S(ERROR) << "Invalid response code: " << res.result() << " body:\n" << res;
    throw std::runtime_error("Bad http response code");
  }

  boost::system::error_code ec;

  pt::ptree data;
  std::stringstream in(res.body.data()); 
  pt::read_json(in, data);
  std::string token = data.get<std::string>("access.token.id");
  LOG_S(INFO) << "Got token: " << token;

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
