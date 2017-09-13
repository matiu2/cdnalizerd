#include "url.hpp"

#include "exception_tags.hpp"

#include <boost/spirit/home/x3.hpp>
#include <sstream>
#include <boost/range/iterator_range.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/vector_tie.hpp>

namespace url_parser {

using namespace boost::spirit::x3;

auto protocol = (string("https") >> "://") | (string("http") >> "://");
auto hostname = +(char_ - (lit(':') | '?' | '/' | eoi));
auto password = +(char_ - (lit('@') | '?' | '/' | eoi)) >> lit('@');
auto user_pass = +(char_ - (lit(':') | '@' | '?' | '/' | eoi)) >>
                 (lit('@') | (':' >> password));
auto port = lit(':') >> ushort_;
auto path = +(char_ - (lit('?') | eoi));
auto key = +(char_ - lit('=')) >> lit('=');
auto val = +(char_ - (lit('&') | eoi)) >> (lit('&') | eoi);
auto spacer = space;

} /* url_parser */ 

namespace cdnalizerd {

void assignURL(URL& destination, const std::string& url) {
  auto begin = url.cbegin();
  auto i = url.cbegin();
  auto end = url.cend();
  auto distance = [&]() { return std::distance(begin, i); };
  using boost::spirit::x3::phrase_parse;
  // Parse the protocol
  bool ok = phrase_parse(i, end, url_parser::protocol, url_parser::spacer,
                         destination.protocol);
  if (!ok) {
    BOOST_THROW_EXCEPTION(
        boost::enable_error_info(std::runtime_error("Unable to parse protocol"))
        << err::action("Reading URL") << err::source(url) << err::position(0));
  }
  // Parse the username and password
  auto user_pass = boost::fusion::vector_tie(destination.username, destination.password);
  ok = phrase_parse(i, end, url_parser::user_pass, url_parser::spacer,
                    user_pass);
  if (!ok) {
    // Because it actually parses whatever it finds into the destination straight away,
    // if it fails, we have to wipe it out
    destination.username.clear();
    destination.password.clear();
  }

  // Parse the hostname
  ok = phrase_parse(i, end, url_parser::hostname, url_parser::spacer,
                    destination.hostname);
  if (!ok) {
    BOOST_THROW_EXCEPTION(
        boost::enable_error_info(std::runtime_error("Unable to parse hostname"))
        << err::action("Reading URL") << err::source(url)
        << err::position(distance()));
  }
  // Parse the port
  ok = phrase_parse(i, end, url_parser::port, url_parser::spacer,
                    destination.port);
  if (!ok) {
    // If no explicit port is set, set it from the protocol
    destination.port = destination.protocol == "https" ? 443 : 80;
  }
  // Parse the path
  phrase_parse(i, end, url_parser::path, url_parser::spacer, destination.path);

  // See if there are any params
  ok = phrase_parse(i, end, url_parser::lit('?'), url_parser::spacer);
  if (ok) {
    while (i != end) {
      std::string key, val;
      // Get each key
      ok = phrase_parse(i, end, url_parser::key, url_parser::spacer, key);
      if (!ok) {
        BOOST_THROW_EXCEPTION(boost::enable_error_info(std::runtime_error(
                                  "Unable to understand get paramater name"))
                              << err::action("Reading URL") << err::source(url)
                              << err::position(distance()));
      }
      // .. and each param
      ok = phrase_parse(i, end, url_parser::val, url_parser::spacer, val);
      if (!ok) {
        BOOST_THROW_EXCEPTION(boost::enable_error_info(std::runtime_error(
                                  "Unable to understand get paramater value"))
                              << err::action("Reading URL") << err::source(url)
                              << err::position(distance()));
      }
      // Now save them
      destination.params.insert(std::make_pair(key, val));
    }
  }

  // Make sure we parsed it all
  if (i != end) {
    BOOST_THROW_EXCEPTION(boost::enable_error_info(std::runtime_error(
                              "Found more URL than expected"))
                          << err::action("Reading URL") << err::source(url)
                          << err::position(distance()));
  }

}

URL::URL(std::string url) {
  assignURL(*this, url);
}

URL &URL::operator=(const std::string &url) {
  // Clear out what's there first, because the parser just appends to the end
  protocol.clear();
  hostname.clear();
  path.clear();
  params.clear();
  assignURL(*this, url);
  return *this;
}

std::string URL::host_part() const {
  std::stringstream out;
  out << protocol << "://" << hostname;
  if (((protocol == "http") && (port != 80)) ||
      ((protocol == "https") && (port != 443))) {
    out << ':' << port;
  }
  return out.str();
}

void getPathPart(const URL& url, std::stringstream& out) {
  out << url.path;
  if (!url.params.empty()) {
    out << '?';
    auto i = url.params.cbegin();
    auto end = url.params.cend();
    --end;
    while (i != end) {
      out << i->first << '=' << i->second << '&';
      ++i;
    }
    out << i->first << '=' << i->second;
  }
}

std::string URL::path_part() const {
  std::stringstream out;
  getPathPart(*this, out);
  return out.str();
}

std::string URL::whole() const {
  std::stringstream out;
  // Protocol
  out << protocol << "://";
  // username ?
  if (!username.empty()) {
    out << username;
    if (!password.empty())
      out << ':' << password;
    out << '@';
  }
  // hostname
  out << hostname;
  if (((protocol == "http") && (port != 80)) ||
      ((protocol == "https") && (port != 443))) {
    out << ':' << port;
  }
  // The path and params
  getPathPart(*this, out);
  return out.str();
}

URL& operator /(URL& a, const std::string& b) {
  std::string& path(a.path);
  if (((!path.empty()) && (path.back() == '/')) ||
      ((!b.empty()) && (b.front() == '/')))
    a.path += b;
  else
    a.path += "/" + b;
  return a;
}

} /* cdnalizerd */ 
