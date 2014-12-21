/// Uses simple system inotify, in a thread and ends it on demand
/// # Documentation sources:
/// http://www.win.tue.nl/~aeb/linux/lk/lk-12.html

#include <sys/inotify.h>
#include <cerrno>
#include <system_error>
#include <vector>
#include <string>

void myThread() {


}

namespace inotify {

class Watch {
private:
  int inotify_handle = 0;
  int handle = -1;
  // Erases all knowledge of our resources
  void erase() {
    inotify_handle = 0;
    handle = -1;
  }
public:
  Watch(int inotify_handle, const char *path, uint32_t mask)
      : inotify_handle(inotify_handle),
        handle(inotify_add_watch(inotify_handle, path, mask)) {
    if (handle == -1)
      throw std::system_error(errno, std::system_category());
  }
  Watch(const Watch& other) = delete; // Can't copy it, it's a real resource
  // Move is fine
  Watch(Watch &&other)
      : inotify_handle(other.inotify_handle), handle(other.handle) {
    other.erase(); // Make the other one forget about our resources, so that
                   // when the destructor is called, it won't try to free them,
                   // as we own them now
  } 
  // Clean up resources on destruction
  ~Watch() {
    if (handle != -1) {
      int result = inotify_rm_watch(inotify_handle, handle);
      if (result == -1)
        throw std::system_error(errno, std::system_category());
    }
  }
  // Move assignment is fine
  Watch &operator=(Watch &&other) {
    inotify_handle = other.inotify_handle;
    handle = other.handle;
    other.erase(); // Make the other one forget about our resources, so that
                   // when the destructor is called, it won't try to free them,
                   // as we own them now
    return *this;
  }
  bool operator ==(const Watch& other) {
    return (inotify_handle == other.inotify_handle) && (handle == other.handle);
  }
};

/// Fire it up to init the inotify queue
struct Instance {
  int handle;
  std::vector<Watch> watches;
  Instance() : handle(inotify_init()) {
    if (handle == -1)
      throw std::system_error(errno, std::system_category());
  }
  Watch& add_watch(const char* path, uint32_t mask) {
    watches.emplace_back(Watch(handle, path, mask));
    return watches.back();
  }
  void remove_watch(Watch &watch) {
    for (auto i = watches.begin(); i != watches.end(); ++i)
      if (*i == watch) {
        watches.erase(i);
        break;
      }
  }
};

}

int main(int argc, char** argv)
{
  std::string dir = "/tmp";
  if (argc == 2)
    dir = argv[1];
  inotify::Instance inotify;
  auto watch = inotify.add_watch(dir, IN_ALL_EVENTS);

  return 0;
}
