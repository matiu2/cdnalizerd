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

#include "utils.hpp"
#include "https.hpp"
#include "logging.hpp"
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
  void die() {
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
    other.die(); // Make the other one forget about our resources, so that
                 // when the destructor is called, it won't try to free them,
                 // as we own them now
  }
  // Clean up resources on destruction
  ~Watch() {
    if (_handle != -1)
      inotify_rm_watch(inotify_handle, _handle);
  }
  // Move assignment is fine
  Watch &operator=(Watch &&other) {
    inotify_handle = other.inotify_handle;
    _handle = other._handle;
    other.die(); // Make the other one forget about our resources, so that
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
    DLOG_S(9) << "Making event - namePtr: " << namePtr << " - len: " << nameLen;
    auto out = std::back_inserter(name);
    while ((*namePtr != '\0') && (namePtr < namePtr + nameLen)) {
      *out = *namePtr;
      ++namePtr;
      ++out;
    }
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
  /// File was closed (could have been written or not)
  bool wasClosed() const {
    return mask & IN_CLOSE;
  }

  /// File was moved, either from or to
  bool wasMoved() const {
    return mask & IN_MOVE;
  } 

  /* special flags */
  bool onlyIfDir() const { return mask & IN_ONLYDIR; }
  bool dontFollow() const { return mask & IN_DONT_FOLLOW; }
  bool excludeEventsOnUnlinkedObjects() const { return mask & IN_EXCL_UNLINK; }
  bool addToTheMask() const { return mask & IN_MASK_ADD; }
  bool isDir() const { return mask & IN_ISDIR; }
  bool oneShot() const { return mask & IN_ONESHOT; }

};

/// Print the event info
std::ostream &operator<<(std::ostream &out, const Event &e);

/// Print an inotify watch
inline std::ostream &operator<<(std::ostream &out, const Watch &watch) {
  out << watch.handle() << " - " << watch.path;
  return out;
}
namespace asio = boost::asio;

/// A collection of Watches
struct Instance {
  yield_context &yield;
  int inotify_handle;
  asio::io_service& ios;
  asio::posix::stream_descriptor stream;
  char buffer[sizeof(inotify_event) + NAME_MAX + 1];
  char* buffer_data_end = buffer;

  // Watch handle to watcher lookup
  std::map<int, Watch> watches;
  // Path to watcher handle lookup
  std::map<std::string, int> paths;

  Instance(yield_context &yield)
      : yield(yield), inotify_handle(inotify_init1(IN_NONBLOCK)),
        ios(service()), stream(ios) {
    if (inotify_handle == -1)
      throw std::system_error(errno, std::system_category());
    stream.assign(inotify_handle);
  }
  const Watch &watchFromHandle(int handle) const { return watches.at(handle); }
  Watch &addWatch(const char *path, uint32_t mask) {
    LOG_S(5) << "Watching path: " << path << " mask(" << std::hex << mask
             << ") inotify handle(" << inotify_handle << ")" << std::endl;
    DLOG_S(9) << "paths: find " << path;
    auto found = paths.find(path);
    if (found != paths.end())
      throw std::logic_error("Can't watch the same path twice");
    auto watch = Watch(inotify_handle, path, mask);
    LOG_S(5) << "watch handle for path (" << path << ") = " << watch.handle()
             << std::endl;
    DLOG_S(9) << "paths: insert " << path << " - " << watch.handle();
    paths.insert({watch.path, watch.handle()});
    DLOG_S(9) << "watches: adding: " << watch;
    auto result =
        watches.emplace(std::make_pair(watch.handle(), std::move(watch)));
    return result.first->second;
  }
  void removeWatch(const Watch &watch) {
    DLOG_S(9) << "watches: find " << watch;
    auto found = watches.find(watch.handle());
    if (found != watches.end()) {
      DLOG_S(9) << "paths: delete " << found->second.path;
      paths.erase(found->second.path);
      DLOG_S(9) << "watches: delete " << watch;
      watches.erase(found);
    }
  }
  void removeWatch(const std::string &path) {
    DLOG_S(9) << "paths: find " << path;
    auto found = paths.find(path);
    if (found != paths.end()) {
      int handle = found->second;
      DLOG_S(9) << "watches: delete " << handle;
      watches.erase(handle);
      DLOG_S(9) << "paths: delete " << found->first;
      paths.erase(found);
    }
  }
  bool alreadyWatching(const std::string &path) const {
    DLOG_S(9) << "paths: find " << path;
    auto found = paths.find(path);
    return (found != paths.end());
  }
  Event waitForEvent() {
    // First check in the buffer if we have anything left over from last time
    LOG_SCOPE_F(9, "Event processing");
    size_t size = std::distance(buffer, buffer_data_end);
    DLOG_S(9) << "Starting buffer size: " << size;
    size_t toRead = sizeof(inotify_event);
    if (size < sizeof(inotify_event)) {
      // If we don't already have enough data in the buffer, read some more
      toRead -= size;
      DLOG_S(9) << "toRead: " << toRead;
      DLOG_S(9) << "NAME_MAX: " << NAME_MAX;
      DLOG_S(9) << "sizeof(inotify_event): " << sizeof(inotify_event);
      size_t maxRead(sizeof(buffer) - size);
      DLOG_S(9) << "Max Read: " << maxRead;
      int bytesRead = boost::asio::async_read(
          stream, asio::buffer(buffer_data_end, maxRead),
          boost::asio::transfer_at_least(toRead), yield);
      DLOG_S(9) << "bytesRead: " << bytesRead;
      buffer_data_end += bytesRead;
      size = std::distance(buffer, buffer_data_end);
      DLOG_S(9) << "New size: " << size;
    }
    assert(size >= sizeof(inotify_event));
    assert(size <= sizeof(buffer));
    inotify_event* event = (inotify_event*)(buffer);
    // Make sure we've read the whole length of the file name
    if (size < (sizeof(inotify_event) + event->len)) {
      toRead = sizeof(inotify_event) + event->len - size;
      DLOG_S(9) << "Getting rest of event name. Buffer size(" << size
                << ") - len(" << event->len << ") - toRead(" << toRead << ")";
      int bytesRead = boost::asio::async_read(
          stream, asio::buffer(buffer_data_end, sizeof(buffer) - size),
          boost::asio::transfer_at_least(toRead), yield);
    }
    DLOG_S(9) << "Raw event: \n"
              << "wd: " << event->wd << "\nmask: " << event->mask
              << "\ncookie: " << event->cookie << "\nlen: " << event->len
              << "\nname: " << event->name << "\n";
    // Now read the event name
    Event result(
        [ this, wd = event->wd ]()->const Watch & { return watches.at(wd); },
        event->mask, event->cookie, event->name, event->len);
    // Now clean up our buffer
    if (size > (sizeof(inotify_event) + event->len)) {
      // Copy / move the next event data to the beginning
      DLOG_S(9) << "Readying buffer for next read";
      size_t start(sizeof(inotify_event) + event->len);
      DLOG_S(9) << "New data found at: " << start;
      std::copy(&buffer[start], buffer_data_end, buffer);
      buffer_data_end -= start;
    } else {
      // Buffer is empty
      DLOG_S(9) << "Buffer is already clean because the buffer is " << size
                << " bytes, and the event is "
                << (sizeof(inotify_event) + event->len);
      buffer_data_end = buffer;
    }
    return result;
  }
};
}
} /* cdnalizerd  */
