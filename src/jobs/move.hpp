#pragma once

#include "../Job.hpp"

#include <fstream>

namespace cdnalizerd {

namespace jobs {

Job makeMoveJob(fs::path source, LUrlParser::clParseURL dest) {
  Job::Worker go =
      [ source = std::move(source), dest = std::move(dest) ](REST & conn) {
    {
      std::ifstream file(source);
      conn.put(dest.path_part()).body(file).go();
    }
    // Delete the source file
    remove(job.dest.c_str());
  };
  return Job("Upload "s + source.string(), go);
}

} /* jobs */ 
} /* cdnalizerd  */ 
