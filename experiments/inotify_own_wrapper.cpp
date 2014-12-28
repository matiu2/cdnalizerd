/// Uses simple system inotify, in a thread and ends it on demand
/// # Documentation sources:
/// http://www.win.tue.nl/~aeb/linux/lk/lk-12.html

#include <sys/inotify.h>
#include <unistd.h>
#include <cerrno>
#include <system_error>
#include <vector>
#include <map>
#include <string>
#include <functional>

namespace inotify {

struct Instance;

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
  Watch(int inotify_handle, const char *path, uint32_t mask)
      : inotify_handle(inotify_handle),
        _handle(inotify_add_watch(inotify_handle, path, mask)) {
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
};

/// Fire it up to init the inotify queue
struct Instance {
  int handle;
  std::map<int, Watch> watches;
  Instance() : handle(inotify_init()) {
    if (handle == -1)
      throw std::system_error(errno, std::system_category());
  }
  Watch& add_watch(const char* path, uint32_t mask) {
    auto watch = Watch(handle, path, mask);
    auto result = watches.emplace(std::make_pair(watch.handle(), std::move(watch)));
    return result.first->second;
  }
  void remove_watch(Watch &watch) {
    auto found = watches.find(watch.handle());
    if (found != watches.end())
      watches.erase(found);
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

int main(int argc, char** argv)
{
  std::string dir = "/tmp";
  if (argc == 2)
    dir = argv[1];
  inotify::Instance inotify;
  auto& watch = inotify.add_watch(dir.c_str(), IN_ALL_EVENTS);
  // TODO: Wait for watches, then act on them

  return 0;
}
