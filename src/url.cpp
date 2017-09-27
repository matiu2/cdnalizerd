#include "url.hpp"

#include <boost/algorithm/string/join.hpp>

// Implements URL::parse - see url_implementation.hpp.rl for real source file
#include "url_parser.hpp"


namespace cdnalizerd {

URL::URL(UnParsedURL in) : raw(boost::algorithm::join(in.parts, "/")) {
  parse();
}

} /* cdnalizerd */ 
