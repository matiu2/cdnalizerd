/// String functions

#include "string_functions.hpp"
#include "errors.hpp"

#include <algorithm>
#include <iterator>

namespace string_fun {

void trim(std::string& data) {
    auto isWs = [](char c) { return (c == ' ') || (c == '\t'); };
    // Find the end
    auto end = std::find_if_not(data.rbegin(), data.rend(), isWs);
    // Erase the end
    if (end != data.rend())
        data.erase(end.base(), data.end());
    // Find the start
    auto start = std::find_if_not(data.begin(), data.end(), isWs);
    // Shift the string forward is the start is different to what it was
    if (start != data.begin())
        data.assign(start, end.base());
}

std::string::const_iterator dequoteString(const std::string& input, std::string& output) {
    auto p = input.cbegin();
    auto out = std::back_inserter(output);
    while (p != input.cend()) {
        switch (*p) {
            case '\\': 
                ++p; // Skip the escape character
                if (p != input.cend())
                    *out++ = *p++;
                break;
            case '"':
                return ++p;          // End of input
            default:
                *out++ = *p++;
        };
    }
    return input.end();
}

}
