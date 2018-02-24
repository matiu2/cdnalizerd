/// Uses simple system inotify, in a thread and ends it on demand
/// # Documentation sources:
/// http://www.win.tue.nl/~aeb/linux/lk/lk-12.html

#include <sys/inotify.h>
#include <unistd.h>
#include <cerrno>
#include <system_error>
#include <string>
#include <functional>
#include <iostream>

#include <boost/asio/spawn.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/buffers_iterator.hpp>

#include "../src/config/config_reader.hpp"

using boost::asio::yield_context;

namespace inotify {

using namespace cdnalizerd;

namespace asio = boost::asio;

int monitor(yield_context yield, asio::io_service& io) {
  // Make the inotify handle (and make asynchronous)
  int handle(inotify_init1(IN_NONBLOCK));
  if (handle == -1)
    throw std::system_error(errno, std::system_category());

  // Wrap it in boost
  asio::posix::stream_descriptor stream(io);
  union {
    char raw[sizeof(inotify_event) + NAME_MAX + 1];
    inotify_event event;
  } data;
  boost::asio::mutable_buffers_1 buffer(asio::buffer(data.raw));
  stream.assign(handle);

  // Now we have at least on event


  // Start watching a path
  std::string path("tmp");
  std::string path2("tmp/xxx");
  int main_handle(inotify_add_watch(
      handle, path.c_str(),
      IN_CLOSE_WRITE |     // File opened for writing was closed (*).
          IN_CREATE |      // File/directory created in watched directory (*).
          IN_DELETE |      // File/directory deleted from watched directory (*).
          IN_DELETE_SELF | // Watched file/directory was itself deleted.
          IN_MODIFY |      // File was modified (*).
          IN_MOVE_SELF |   // Watched file/directory was itself moved.
          IN_MOVED_FROM |  // File moved out of watched directory (*).
          IN_MOVED_TO      // File moved into watched directory (*).
      ));
  int handle2(inotify_add_watch(
      handle, path2.c_str(),
      IN_CLOSE_WRITE |     // File opened for writing was closed (*).
          IN_CREATE |      // File/directory created in watched directory (*).
          IN_DELETE |      // File/directory deleted from watched directory (*).
          IN_DELETE_SELF | // Watched file/directory was itself deleted.
          IN_MODIFY |      // File was modified (*).
          IN_MOVE_SELF |   // Watched file/directory was itself moved.
          IN_MOVED_FROM |  // File moved out of watched directory (*).
          IN_MOVED_TO      // File moved into watched directory (*).
      ));


  while (true) {
    // Read some data
    size_t count =
        asio::async_read(stream, buffer,
                         asio::transfer_at_least(sizeof(inotify_event)), yield);
    std::cout << "Read " << count << " bytes" << std::endl;
    std::cout << "Event" << '\n'
              << "  wd: " << data.event.wd << '\n'
              << "  mask: " << std::hex << data.event.mask << '\n'
              << "  cookie: " << std::hex << data.event.cookie << '\n'
              << "  let: " << data.event.len << '\n'
              << "  name: " << data.event.name << '\n'
              << std::endl;
  }

  std::cout << "DONE" << std::endl;

  inotify_rm_watch(handle, handle2);
  inotify_rm_watch(handle, main_handle);
  return 0;
}
}



int main(int argc, char **argv) {
  boost::asio::io_service io;
  int result;
  boost::asio::spawn(io, [&result, &io](boost::asio::yield_context y) { result = inotify::monitor(y, io); });
  io.run();
  return result;
}
