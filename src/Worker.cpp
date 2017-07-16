#include "Worker.hpp"

#include <RESTClient/rest.hpp>
#include "processes/uploader.hpp"

namespace cdnalizerd {

using RESTClient::http::URL;
using RESTClient::http::Response;
using RESTClient::REST;

void Worker::launch(std::function<void()> onDone) {
  assert(onDone);
  _onDone = onDone;
  _state = Ready;
  RESTClient::http::spawn(
      [this](yield_context y) { processes::uploader(y, *this); });
}

} /* cdnalizerd  */ 
