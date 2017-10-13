#pragma once

#include <string>
#include <vector>

namespace cdnalizerd {

/// Add a regex filter of files to ignore
void addFilter(std::string filter);

/// Returns true if a file should be ignored
bool shouldIgnoreFile(const std::string& fileName);

} /* cdnalizerd  */ 
