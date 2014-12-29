/// Our own c++ wrapper around inotify. Build from ../experiments/inotify_own_wrapper.cpp

#include <sys/inotify.h>
#include <unistd.h>
#include <system_error>

namespace inotify {

/// A watch on a single directory
class Watch {
private:
  int inotify_handle = 0;
  int _handle = -1;
  // Erases all knowledge of our resources
  void erase() {
    inotify_handle = 0;
    _handle = -1;
  }
public:
  const char* path;
  Watch(int inotify_handle, const char *path, uint32_t mask)
      : inotify_handle(inotify_handle),
        _handle(inotify_add_watch(inotify_handle, path, mask)), path(path) {
    if (_handle == -1)
      throw std::system_error(errno, std::system_category());
  }
  Watch(const Watch& other) = delete; // Can't copy it, it's a real resource
  // Move is fine
  Watch(Watch &&other)
      : inotify_handle(other.inotify_handle), _handle(other._handle) {
    other.erase(); // Make the other one forget about our resources, so that
                   // when the destructor is called, it won't try to free them,
                   // as we own them now
  } 
  // Clean up resources on destruction
  ~Watch() {
    if (_handle != -1) {
      int result = inotify_rm_watch(inotify_handle, _handle);
      if (result == -1)
        throw std::system_error(errno, std::system_category());
    }
  }
  // Move assignment is fine
  Watch &operator=(Watch &&other) {
    inotify_handle = other.inotify_handle;
    _handle = other._handle;
    other.erase(); // Make the other one forget about our resources, so that
                   // when the destructor is called, it won't try to free them,
                   // as we own them now
    return *this;
  }
  bool operator ==(const Watch& other) {
    return (inotify_handle == other.inotify_handle) && (_handle == other._handle);
  }
  int handle() const { return _handle; }
};

struct Event {
    Watch&   watch;
    uint32_t mask;     /* Mask of events */
    uint32_t cookie;   /* Unique cookie associating related
                          events (for rename(2)) */
    std::string name;

    std::string path() const {
      auto out = watch.path;
      joinPaths(out, name);
      return out;
    }
    
    /* Events we can watch for */
    bool wasAccessed() const { return mask & IN_ACCESS; }
    bool wasModified() const { return mask & IN_MODIFY; }
    bool wasChanged() const { return mask & IN_ATTRIB; }
    bool wasSaved() const { return mask & IN_CLOSE_WRITE; }
    bool wasClosedWithoutSave() const { return mask & IN_CLOSE_NOWRITE; }
    bool wasOpened() const { return mask & IN_OPEN; }
    bool wasMovedFrom() const { return mask & IN_MOVED_FROM; }
    bool wasMovedTo() const { return mask & IN_MOVED_TO; }
    bool wasCreated() const { return mask & IN_CREATE; }
    bool wasDeleted() const { return mask & IN_DELETE; }
    bool wasSelfDeleted() const { return mask & IN_DELETE_SELF; } // This means our actual directory was deleted
    bool wasSelfMoved() const { return mask & IN_MOVE_SELF; } // TODO: Test if we need to re-set up watches after *self* was moved

    /* Events we get wether we like it or not  */
    bool wasUnmounted() const { return mask & IN_UNMOUNT; }
    bool wasOverflowed() const { return mask & IN_Q_OVERFLOW; }
    bool wasIgnored() const { return mask & IN_IGNORED; }

    /* Helper events */
    bool wasClose() const { return mask & IN_CLOSE; } /// File was closed (could have been written or not)
    bool wasMoved() const { return mask & IN_MOVE; }  /// File was moved, either from or to

    /* special flags */
    bool onlyIfDir() const { return mask & IN_ONLYDIR; }
    bool dontFollow() const { return mask & IN_DONT_FOLLOW; }
    bool excludeEventsOnUnlinkedObjects() const { return mask & IN_EXCL_UNLINK; }
    bool addToTheMask() const { return mask & IN_MASK_ADD; }
    bool isDir() const { return mask & IN_ISDIR; }
    bool oneShot() const { return mask & IN_ONESHOT; }
};

/// A collection of Watches
struct Instance {
  int handle;
  std::map<int, Watch> watches;
  std::vector<std::string, Watch*> paths;
  Instance() : handle(inotify_init()) {
    if (handle == -1)
      throw std::system_error(errno, std::system_category());
  }
  Watch& addWatch(const char* path, uint32_t mask) {
    auto found = paths.find(path);
    if (found != paths.end())
      throw std::logic_error("Can't watch the same path twice");
    auto watch = Watch(handle, path, mask);
    auto result = watches.emplace(std::make_pair(watch.handle(), std::move(watch)));
    paths.insert({watch.path, &watch});
    return result.first->second;
  }
  void removeWatch(Watch &watch) {
    auto found = watches.find(watch.handle());
    if (found != watches.end())
      watches.erase(found);
  }
  void removeWatch(const std::string& path) {
    auto found = paths.find(path);
    if (found != paths.end())
      paths.erase(found);
  }
  bool alreadyWatching(const std::string& path) const {
    auto found = paths.find(path);
    return (found != paths.end());
  }
  std::vector<Event> waitForEvents() {
    int len;
    constexpr int buf_size = 1024; // TODO: Work out a better number. Maybe put it in the config
    char events[buf_size];
    // This is where the program will pause, until a signal is received (and
    // errno=EINTR) or a file is changed
    len = read(handle, events, buf_size);
    if (len == -1)
      throw std::system_error(errno, std::system_category());
    std::vector<Event> result;
    inotify_event* event = reinterpret_cast<inotify_event*>(events);
    while (reinterpret_cast<void*>(event) < events + len) {
      result.emplace_back(Event{watches.at(event->wd), event->mask, event->cookie});
      Event& out = result.back();
      out.name.reserve(event->len);
      std::copy(event->name, &event->name[event->len], std::back_inserter(out.name));
      // Get the next event
      event += sizeof(inotify_event) + event->len;
    }
    return result;
  }
};

}
