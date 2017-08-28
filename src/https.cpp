#include "https.hpp"

#include <cassert>

namespace cdnalizerd {

thread_local asio::io_service* _global_ios(nullptr);

void service(asio::io_service* ios) {
  assert(ios);
  _global_ios = ios;
}

asio::io_service& service() {
  assert(_global_ios);
  return *_global_ios;
}
  
} /* cdnalizerd  */ 
