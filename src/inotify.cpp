#include "inotify.hpp"

namespace cdnalizerd {
namespace inotify {
  
std::ostream &operator<<(std::ostream &out, const Event &e) {
  using namespace std;
  cout << "Event: path(" << e.path() << ") ";
  if (e.wasAccessed())
    cout << " Accessed ";
  if (e.wasModified())
    cout << " Modified ";
  if (e.wasChanged())
    cout << " Changed ";
  if (e.wasSaved())
    cout << " Saved ";
  if (e.wasClosedWithoutSave())
    cout << " ClosedWithoutSave ";
  if (e.wasOpened())
    cout << " Opened ";
  if (e.wasMovedFrom())
    cout << " MovedFrom ";
  if (e.wasMovedTo())
    cout << " MovedTo ";
  if (e.wasCreated())
    cout << " Created ";
  if (e.wasDeleted())
    cout << " Deleted ";
  if (e.wasSelfDeleted())
    cout << " SelfDeleted ";
  if (e.wasSelfMoved())
    cout << " SelfMoved ";
  if (e.onlyIfDir())
    cout << " onlyIfDir ";
  if (e.dontFollow())
    cout << " dontFollow ";
  if (e.excludeEventsOnUnlinkedObjects())
    cout << " excludeEventsOnUnlinkedObjects ";
  if (e.addToTheMask())
    cout << " addToTheMask ";
  if (e.isDir())
    cout << " isDir ";
  if (e.oneShot())
    cout << " oneShot ";
  return out;
}

}
} /* cdnalizerd  */ 
