#include "url.hpp"

#include <boost/algorithm/string/join.hpp>

#include "url_parser.hpp"


namespace cdnalizerd {

URL::URL(UnParsedURL in) : raw(boost::algorithm::join(in.parts, "/")) {
  parse();
}

} /* cdnalizerd */ 
