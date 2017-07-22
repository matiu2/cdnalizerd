#pragma once

#include <string>

namespace cdnalizerd {

/// Writes out all the entries. If there are more than one entry; filename is
/// a directory and we'll output .info files.  If there is only one entry, we'll
/// write it out by whatever filename it has
void write_sample_config(const std::string& filename);
  
} /* cdnalizerd  */ 
