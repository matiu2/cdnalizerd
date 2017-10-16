#include "url.hpp"

#include <boost/algorithm/string/join.hpp>

#include "url_parser.hpp"


namespace cdnalizerd {

// Returns one big string where with all mini strings joined by a single slash
std::string joinSlashes(std::vector<std::string>& strings) {
  std::string result;
  size_t size = 0;
  for (const auto& s : strings )
    size += s.size() + 1;
  result.reserve(size);
  auto out = std::back_inserter(result);
  for (const auto& s : strings ) {
    if (s.empty())
      continue;
    auto begin = s.begin();
    if (s.front() == '/')
      ++begin;
    auto end = s.end();
    if (s.back() == '/')
      --end;
    std::copy(begin, end, out);
    *out++ = '/';
  }
  result.erase(result.end() - 1);
  return result;
}

URL::URL(UnParsedURL in) : raw(joinSlashes(in.parts)) { parse(); }

/// Takes a filename or path and url encodes it
std::string urlencode(const std::string &path) {
  std::string result;
  result.reserve(path.size() * 1.05); // Estamite 5% growth
  auto out = std::back_inserter(result);
  for (unsigned char in : path) {
    // Legal statements - a-z A-Z 0-9 . - _ ~ ! $ & ' ( ) * + , ; = : @
    if (((in >= 'a') && (in <= 'z')) || ((in >= 'A') && (in <= 'Z')) ||
        ((in >= '0') && (in <= '9'))) {
      *out = in;
      ++out;
    } else {
      switch (in) {
      case '.':
      case '-':
      case '_':
      case '~':
      case '!':
      case '$':
      case '&':
      case '\'':
      case '(':
      case ')':
      case '*':
      case '+':
      case ',':
      case ';':
      case '=':
      case ':':
      case '/':
      case '@': {
        *out = in;
        ++out;
        break;
      }
      default: {
        DLOG_S(9) << "Converting to hex: " << in;
        *out = '%';
        ++out;
        unsigned char outChar;
        unsigned char left = in >> 4;
        if (left <= 9)
          outChar = '0' + left;
        else
          outChar = 'A' + left - 0x0A;
        DLOG_S(9) << "hex left: " << outChar;
        *out = outChar;
        ++out;
        unsigned char right = in & 0x0F;
        if (right <= 9)
          outChar = '0' + right;
        else
          outChar = 'A' + right - 0x0A;
        DLOG_S(9) << "hex right: " << outChar;
        *out = outChar;
        ++out;
      }
      };
    }
  }
  return result;
}

} /* cdnalizerd */ 
