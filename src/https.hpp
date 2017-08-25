#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/spawn.hpp>

namespace cdnalizerd {

namespace asio = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

struct Services {
};

class HTTPS {
public:
  using Stream = ssl::stream<asio::ip::tcp::socket &>;
private:
  asio::yield_context &yield;
  ssl::context ctx;
  tcp::socket sock;
  std::unique_ptr<Stream> s;

public:
  HTTPS(asio::yield_context &yield, asio::io_service &ios,
        std::string hostname, std::string service = "https")
      : yield(yield), ctx(ssl::context::tlsv12), sock(ios) {
    tcp::resolver dns{ios};
    ctx.set_default_verify_paths();
    auto const lookup = dns.async_resolve({hostname, "https"}, yield);
    asio::async_connect(sock, lookup, yield);
    sock.set_option(tcp::no_delay(true));
    s.reset(new Stream(sock, ctx));
    s->set_verify_mode(ssl::verify_peer);
    s->set_verify_callback(ssl::rfc2818_verification(hostname));
    s->handshake(Stream::client);
  }
  ~HTTPS() {
    boost::system::error_code ec;
    if (!s)
      return;
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
    // Something scary happened, log an error (throw an exception too)
    LOG_S(FATAL) << "Unable to close down SSL connection";
  }
  Stream &stream() {
    assert(s);
    return *s;
  }
};

} /* cdnalizerd  */ 
