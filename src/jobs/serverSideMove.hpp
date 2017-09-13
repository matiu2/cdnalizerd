#pragma once

#include "../Job.hpp"
#include "../exception_tags.hpp"

namespace cdnalizerd {

namespace jobs {

Job makeServerSideMove(URL source, URL dest) {
  Job::Worker go =
      [ source = std::move(source), dest = std::move(dest) ](REST & conn) {
      if (source.hostname != dest.hostname) {
        // TODO: support a download, then upload copy ?
        BOOST_THROW_EXCEPTION(boost::enable_error_info(std::runtime_error(
                                  "We don't support server side copy between "
                                  "two different hosts"))
                              << err::source(source) << err::destination(dest));
      }
      conn.put(dest.path_part()).add_header("X-Copy-From", source.path_part()).go();
      conn.delet(source.path_part()).go();
  };
  return Job("Server side move from "s + source + " to " + dest, go);
}

} /* jobs */ 
} /* cdnalizerd  */ 

