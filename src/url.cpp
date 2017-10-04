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

} /* cdnalizerd */ 
