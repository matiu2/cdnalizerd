#pragma once

#include <istream>
#include <ostream>
#include <functional>

#include "config.hpp"

namespace cdnalizerd {

/** Reads in a config file.
 *  Every line can be of one of these 4 formats
 *  username=xx
 *  key=yyy
 *  container=zzz
 *  /local/path /remote/path
 *
 *  The settings lines affect all lines below them.
 *
 *  Further examples:
 *
 *  /local/path2 /remote/path2
 *  container=container2
 *  "/local/path with spaces" "/remote/path with spaces"
 *
 *  The last path will be in container2 instead of zzz, but will use the same auth settings
 *
 **/ 
Config read_config(std::istream& in);

}
