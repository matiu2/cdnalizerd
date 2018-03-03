#pragma once

#include <string>

struct Options {
  std::string source;
  std::string dest;
  std::string username;
  std::string apikey;
  std::string region;
  bool snet = false;
  int log_verbosity = 1;
};

/// Set up and parse the command line options for cfsync
bool setupCommandlineOptions(int argc, char **argv, Options &result);
