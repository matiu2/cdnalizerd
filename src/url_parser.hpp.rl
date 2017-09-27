#pragma once

#include "logging.hpp"

namespace {
%%{
    machine url;

    include url "url_actions.rl";
    include url "url.rl";

    write data;
}%%
}

namespace cdnalizerd {

void URL::parse() {
  auto p = std::begin(raw);
  auto pe = std::end(raw);
  auto eof = pe;
  auto start = raw.begin();
  int cs;
  // init
  %%write init;
  // exec
  %%write exec;
  // finish
  if (cs < url_first_final) {
    BOOST_THROW_EXCEPTION(
        boost::enable_error_info(std::runtime_error("Unable to parse URL"))
        << err::URL(raw) << err::position(std::distance(std::begin(raw), p)));
  }
  pathAndSearch = path + search;
}
  
} /* cdnalizerd  */ 
