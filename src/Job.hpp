#pragma once

#include <string>
#include <memory>

namespace cdnalizerd {

enum Operation {
  Upload,  // Upload from local source to url dest
  Move,    // Upload from local source to url dest, then delete local source
  SCopy,    // Server side copy from url source to url dest
  SDelete, // Server side Delete url source (ignore dest)
};

struct Job {
  Operation operation;
  // source is usually the server side path
  std::string source;
  // dest is usually the local path, depnending on the operation
  std::string dest;
  // If set, it should be followed by the next job immediately. 
  // eg. server side copy followed by server side delete.
  // eg. upload to one account, then delete from the other account
  std::shared_ptr<Job> next = {};
  const std::string& workerURL() const {
    switch (operation) {
      case Upload:
      case Move:
        return dest;
      case SCopy:
      case SDelete:
        return source;
    };
  }
};

  
} /* cdnalizerd  */ 
