/// String functions

#include "string_functions.hpp"
#include "errors.hpp"

#include <algorithm>
#include <iterator>

namespace string_fun {

void trim(std::string &data) {
  auto isWs = [](char c) { return (c == ' ') || (c == '\t'); };
  // Find the end
  auto end = std::find_if_not(data.rbegin(), data.rend(), isWs);
  // Erase the end
  if (end != data.rend())
    data.erase(end.base(), data.end());
  // Find the start
  auto start = std::find_if_not(data.begin(), data.end(), isWs);
  // Shift the string forward is the start is different to what it was
  if (start != data.begin()) {
    if (end == data.rend()) {
      data.resize(0);
    } else {
      data.assign(start, end.base());
    }
  }
}

P dequoteString(P start, P end, std::string &output) {
  auto p = start;
  auto out = std::back_inserter(output);
  while (p != end) {
    switch (*p) {
    case '\\':
      ++p; // Skip the escape character
      if (p != end)
        *out++ = *p++;
      else
        *out++ = *p; // If the last character is a backspace, just return it
      break;
    case '"':
      return ++p; // End of input
    default:
      *out++ = *p++;
    };
  }
  return end;
}
}
