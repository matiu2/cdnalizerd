#pragma once
/// Our own c++ wrapper around inotify. Created from
/// ../experiments/inotify_own_wrapper.cpp

#include <sys/inotify.h>
#include <unistd.h>
#include <system_error>
#include <ios>

#include <boost/asio/buffers_iterator.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/optional.hpp>
#include <boost/filesystem.hpp>

#include <RESTClient/tcpip/interface.hpp>

#include "utils.hpp"
#include "config_reader/config.hpp"

namespace cdnalizerd {
  
namespace inotify {

namespace fs = boost::filesystem;

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
  std::string path;
  Watch(int inotify_handle, std::string path, uint32_t mask)
      : inotify_handle(inotify_handle),
        _handle(inotify_add_watch(inotify_handle, path.c_str(), mask)),
        path(std::move(path)) {
    if (_handle == -1)
      throw std::system_error(errno, std::system_category());
  }
  Watch(const Watch &other) = delete; // Can't copy it, it's a real resource
  // Move is fine
  Watch(Watch &&other)
      : inotify_handle(other.inotify_handle), _handle(other._handle),
        path(std::move(other.path)) {
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
  bool operator==(const Watch &other) {
    return (inotify_handle == other.inotify_handle) &&
           (_handle == other._handle);
  }
  int handle() const { return _handle; }
};

/// An event that happened to a file
struct Event {
  using GetWatch = std::function<const Watch &(void)>;
  GetWatch watch; // Is filled in in waitForEvent
  uint32_t mask;   /* Mask of events */
  uint32_t cookie; /* Unique cookie associating related
                      events (for rename(2)) */
  std::string name;
  // If it's a move or copy operation, 'destination' is the destination event
  std::unique_ptr<Event> destination;

  Event(GetWatch watch, uint32_t mask, uint32_t cookie,
        const char *namePtr, int nameLen)
      : watch(watch), mask(mask), cookie(cookie) {
    name.reserve(nameLen);
    std::copy(namePtr, &namePtr[nameLen], std::back_inserter(name));
  }

  std::string path() const {
    fs::path out = watch().path;
    return (out / name).native();
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
  bool wasSelfDeleted() const {
    return mask & IN_DELETE_SELF;
  } // This means our actual directory was deleted
  bool wasSelfMoved() const {
    return mask & IN_MOVE_SELF;
  }

  /* Events we get wether we like it or not  */
  bool wasUnmounted() const { return mask & IN_UNMOUNT; }
  bool wasOverflowed() const { return mask & IN_Q_OVERFLOW; }
  bool wasIgnored() const { return mask & IN_IGNORED; }

  /* Helper events */
  bool wasClose() const {
    return mask & IN_CLOSE;
  } /// File was closed (could have been written or not)
  bool wasMoved() const {
    return mask & IN_MOVE;
  } /// File was moved, either from or to

  /* special flags */
  bool onlyIfDir() const { return mask & IN_ONLYDIR; }
  bool dontFollow() const { return mask & IN_DONT_FOLLOW; }
  bool excludeEventsOnUnlinkedObjects() const { return mask & IN_EXCL_UNLINK; }
  bool addToTheMask() const { return mask & IN_MASK_ADD; }
  bool isDir() const { return mask & IN_ISDIR; }
  bool oneShot() const { return mask & IN_ONESHOT; }

};

/// Print the event info
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

namespace asio = boost::asio;

union RawEvent {
  char raw[sizeof(inotify_event) + NAME_MAX + 1];
  inotify_event event;
};

/// A collection of Watches
struct Instance {
  yield_context &yield;
  int inotify_handle;
  std::shared_ptr<RESTClient::tcpip::io_service> io;
  asio::posix::stream_descriptor stream;
  boost::asio::mutable_buffers_1 buffer;

  // Watch handle to watcher lookup
  std::map<int, Watch> watches;
  // Path to watcher handle lookup
  std::map<std::string, int> paths;

  // Where incoming events are read to
  RawEvent data;

  Instance(yield_context &yield)
      : yield(yield), inotify_handle(inotify_init1(IN_NONBLOCK)),
        io(RESTClient::tcpip::getService()), stream(*io),
        buffer(asio::buffer(data.raw)) {
    if (inotify_handle == -1)
      throw std::system_error(errno, std::system_category());
    stream.assign(inotify_handle);
  }
  const Watch &watchFromHandle(int handle) const { return watches.at(handle); }
  Watch &addWatch(const char *path, uint32_t mask) {
    BOOST_LOG_TRIVIAL(debug) << "Watching path: " << path << " mask("
                             << std::hex << mask << ") inotify handle("
                             << inotify_handle << ")";
    auto found = paths.find(path);
    if (found != paths.end())
      throw std::logic_error("Can't watch the same path twice");
    auto watch = Watch(inotify_handle, path, mask);
    BOOST_LOG_TRIVIAL(debug) << "watch handle for path (" << path
                             << ") = " << watch.handle();
    paths.insert({watch.path, watch.handle()});
    auto result =
        watches.emplace(std::make_pair(watch.handle(), std::move(watch)));
    return result.first->second;
  }
  void removeWatch(Watch &watch) {
    BOOST_LOG_TRIVIAL(debug) << "Removing watch: " << watch.path;
    auto found = watches.find(watch.handle());
    if (found != watches.end()) {
      paths.erase(found->second.path);
      watches.erase(found);
    }
  }
  void removeWatch(const std::string &path) {
    auto found = paths.find(path);
    if (found != paths.end()) {
      int handle = found->second;
      watches.erase(handle);
      paths.erase(found);
    }
  }
  bool alreadyWatching(const std::string &path) const {
    auto found = paths.find(path);
    return (found != paths.end());
  }
  Event waitForEvent() {
    // Get one event
    union EventHolder {
      char raw[sizeof(inotify_event) + NAME_MAX + 1];
      inotify_event event;
    } data;
    int len = boost::asio::async_read(
        stream, boost::asio::buffer(data.raw),
        boost::asio::transfer_at_least(sizeof(inotify_event)), yield);
    if (len == -1)
      throw std::system_error(errno, std::system_category());
    return Event([ this, wd = data.event.wd ]()->const Watch & {
      return watches.at(wd);
    },
                 data.event.mask, data.event.cookie, data.event.name,
                 data.event.len);
  }
};
}
} /* cdnalizerd  */
