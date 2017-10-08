#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/beast.hpp>

#include "logging.hpp"
#include "exception_tags.hpp"
#include "version.hpp"

namespace cdnalizerd {

namespace asio = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

// This is included just for convenient usage
namespace beast = boost::beast;
namespace http = boost::beast::http;

void service(asio::io_service* ios);
asio::io_service& service();

class HTTPS {
public:
  using Stream = ssl::stream<asio::ip::tcp::socket &>;
private:
  asio::io_service &ios;
  ssl::context ctx;
  tcp::socket sock;
  std::unique_ptr<Stream> s;
  std::string hostname;

public:
  asio::yield_context &yield;
  boost::beast::flat_buffer read_buffer;
  void connect() {
    tcp::resolver dns{ios};
    auto const lookup = dns.async_resolve({hostname, "https"}, yield);
    asio::async_connect(sock, lookup, yield);
    sock.set_option(tcp::no_delay(true));
    s.reset(new Stream(sock, ctx));
    s->set_verify_mode(ssl::verify_peer);
    s->set_verify_callback(ssl::rfc2818_verification(hostname));
    s->handshake(Stream::client);
  }
  void disconnect() {
    DLOG_S(9) << "Shutting down https connection: " << hostname;
    boost::system::error_code ec;
    s->async_shutdown(yield[ec]);
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
      return;
    }
    // We are the first one to run ssl_shutdown, and remote party responded in
    // kind, just continue
    if (ec.category() == misc_cat && ec.value() == misc_errors::eof) {
      return;
    }
    // The remote party sent ssl_shutdown, then just dropped the connection
    if (ec.category() == misc_cat &&
        ec.value() == basic_errors::operation_aborted) {
      return;
    }
    // Everything went as planned
    if (ec.category() == boost::system::system_category() &&
        ec.value() == boost::system::errc::success) {
      return;
    }
    // Something scary happened, throw an exception
    LOG_S(WARNING) << "Unable to shut down SSL for " << hostname
                   << " error code: " << ec.value()
                   << " error category: " << ec.category().name()
                   << " error message: " << ec.message();
  }

public:
  HTTPS(asio::yield_context &yield, std::string hostname)
      : ios(cdnalizerd::service()), yield(yield), ctx(ssl::context::tlsv12),
        sock(ios), hostname(hostname) {
    ctx.set_default_verify_paths();
    connect();
  }
  ~HTTPS() {
    if (!s)
      return;
    disconnect();
  }
  Stream &stream() {
    assert(s);
    return *s;
  }
  void reconnect() {
    disconnect();
    // Clean out the read buffer so it doesn't affect future reads
    read_buffer.consume(read_buffer.size()); 
    connect();
  }
};

template <typename Req>
void setDefaultHeaders(Req& req, std::string token) {
    req.version = 11;
    req.set(http::field::user_agent, userAgent());
    req.set(http::field::content_type, "application/json");
    req.set(http::field::accept, "application/json");
    req.set("X-Auth-Token", token);
}


} /* cdnalizerd  */ 
