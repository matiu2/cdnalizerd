#include "queueWorker.hpp"

#include "../Rackspace.hpp"
#include "../Status.hpp"
#include "../config_reader/config.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>

#include <RESTClient/rest.hpp>

namespace cdnalizerd {
namespace processes {

// Struct that removes workers from their list when we're done with them
struct WorkerSentry {
  Worker& worker;
  WorkerSentry(Worker& worker) : worker(worker) {}
  ~WorkerSentry() {
    worker.die();
  }
};

/// Sets the shared string for the token to empty, then if you don't fill it, it deletes it
struct SStringSentry {
  sstring& value;
  SStringSentry(sstring &value) : value(value) {
    assert(!value); // Should start off unset
    value.reset(new std::string());
  }
  ~SStringSentry() {
    // You shouldn't unset the value while we're holding it
    assert(value);
    if (value->empty())
      value.reset();
  }
};

void queueWorker(yield_context &yield, Status& status, Worker& worker) {
// Find which worker wants this job
  WorkerSentry sentry(worker);
  assert(worker.jobsToDo.size() != 0);
  auto job = worker.jobsToDo.begin();


  while (job != worker.jobsToDo.end()) {
    job->configuration.username;
  }
}

} /* processes */ 
} /* cdnalizerd  */ 
