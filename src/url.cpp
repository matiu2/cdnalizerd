#include "url.hpp"

// Implements URL::parse - see url_implementation.hpp.rl for real source file
#include "url_parser.hpp"

namespace cdnalizerd {

UnParsedURL joinStrings(const std::string& a, const std::string& b) {
  bool aEndsWithSlash = (!a.empty()) && (a.back() == '/');
  bool bStartsWithSlash = (!b.empty()) && (b.front() == '/');
  UnParsedURL result(a);
  if (aEndsWithSlash && bStartsWithSlash)
    std::copy(b.begin() + 1, b.end(), std::back_inserter(result.raw));
  else if (aEndsWithSlash || bStartsWithSlash)
    result.raw.append(b);
  else {
    result.raw.append("/");
    result.raw.append(b);
  }
  return result;
}

} /* cdnalizerd */ 
