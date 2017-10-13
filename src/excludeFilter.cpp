#include "excludeFilter.hpp"

#include <algorithm>
#include <regex>

namespace {
  std::vector<std::regex> regexes;
}

namespace cdnalizerd {

// A part is just a substring, eg '/cache/' would match all files containing 'cache'
void addFilter(std::string filter) {
  regexes.emplace_back(std::regex(std::move(filter)));
}

/// Returns true if a file should be ignored
bool shouldIgnoreFile(const std::string& fileName) {
  // Search through each list
  for (const std::regex& regex : regexes) {
    if (std::regex_search(fileName, regex, std::regex_constants::match_any))
      return true;
  }
  return false;
}
  
} /* cdnalizerd */ 
