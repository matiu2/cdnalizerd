#include "inotify.hpp"

namespace cdnalizerd {
namespace inotify {
  
std::ostream &operator<<(std::ostream &out, const Event &e) {
  using namespace std;
  out << "Event: path(" << e.path() << ") ";
  if (e.wasAccessed())
    out << " Accessed ";
  if (e.wasModified())
    out << " Modified ";
  if (e.wasChanged())
    out << " Changed ";
  if (e.wasSaved())
    out << " Saved ";
  if (e.wasClosedWithoutSave())
    out << " ClosedWithoutSave ";
  if (e.wasOpened())
    out << " Opened ";
  if (e.wasMovedFrom())
    out << " MovedFrom ";
  if (e.wasMovedTo())
    out << " MovedTo ";
  if (e.wasCreated())
    out << " Created ";
  if (e.wasDeleted())
    out << " Deleted ";
  if (e.wasSelfDeleted())
    out << " SelfDeleted ";
  if (e.wasSelfMoved())
    out << " SelfMoved ";
  if (e.onlyIfDir())
    out << " onlyIfDir ";
  if (e.dontFollow())
    out << " dontFollow ";
  if (e.excludeEventsOnUnlinkedObjects())
    out << " excludeEventsOnUnlinkedObjects ";
  if (e.addToTheMask())
    out << " addToTheMask ";
  if (e.isDir())
    out << " isDir ";
  if (e.oneShot())
    out << " oneShot ";
  return out;
}

}
} /* cdnalizerd  */ 
