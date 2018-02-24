#include <algorithm>
#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <vector>

#include "config/config.hpp"

std::vector<std::string> input{"abc",
                               "def",
                               "/some/dir/with/abc",
                               "/some/dir/without",
                               "a/b/c/d",
                               "/some/dir/abc/in/the/middle",
                               "/definitely/a/file",
                               "/file/for/fun",
                               "funtastic",
                               "fantastic"};

std::map<std::string, std::vector<std::string>> regexToExpected{
    {"abc", {"abc", "/some/dir/with/abc", "/some/dir/abc/in/the/middle"}},
    {"fun", {"/file/for/fun", "funtastic"}},
    {"^fun", {"funtastic"}},
    {"a.*b.*c",
     {"abc", "/some/dir/with/abc", "a/b/c/d", "/some/dir/abc/in/the/middle"}},
    {"abc$", {"abc", "/some/dir/with/abc"}},
    {"^f.n", {"funtastic", "fantastic"}}
};

// Reads a bunch of regexes from the command line, then uses them to grep
// std::in to std::out, removing files that match
int main(int argc, char *argv[]) {
  if (argc == 1) {
    std::cout << "List regexes to test in the command line. Options are: ";
    for (const auto pair : regexToExpected)
      std::cout << pair.first << " ";
    std::cout << std::endl;
    return 1;
  }  
  cdnalizerd::ConfigEntry e;
  // Check the ones past on the command line
  int result = 0;
  std::vector<std::string> toCheck(argv, argv + argc);
  for (auto i = toCheck.begin() + 1; i < toCheck.end(); ++i) {
    const std::string &regex(*i);
      e.filesToIgnore.emplace_back(std::regex(regex));
      std::vector<std::string> output;
      for (const std::string &line : input)
        if (e.shouldIgnoreFile(line))
          output.push_back(line);
      // Check if the output matches expected
      auto const &expected = regexToExpected.at(regex);
      if (expected != output) {
        ++result;
        std::cerr << "For regex " << regex << "\n --- expected: \n";
        for (const std::string &ex : expected)
          std::cerr << ex << std::endl;
        std::cerr << "\n --- Got: \n";
        for (const std::string &got : output)
          std::cerr << got << std::endl;
    }
  }
  return result;
}
