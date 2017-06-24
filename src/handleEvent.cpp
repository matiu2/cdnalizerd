#include "handleEvent.hpp"

namespace cdnalizerd {

void uploadFile(inotify::Event &event) {
}

void renameFile(inotify::Event &event) {
}

void deleteFile(inotify::Event &event) {
}

void handleEvent(inotify::Event &&event) {
  switch (event.mask) {
  case IN_CLOSE_WRITE: {
    uploadFile(event);
    break;
  }
  case IN_MOVED_FROM: {
    if (event.destination)
      renameFile(event);
    else
      deleteFile(event);
    break;
  }
  case IN_MOVED_TO: {
    // When a file is moved to a directory, for it to appear here, it will have
    // come from outside of our watch space
    assert(!event.destination);
    uploadFile(event);
    break;
  }
  case IN_DELETE: {
    deleteFile(event);
    break;
  }
  };
}

} /* cdnalizerd  */ 
