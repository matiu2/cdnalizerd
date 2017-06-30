#include "queueWorker.hpp"

#include "../Rackspace.hpp"
#include "../Status.hpp"

namespace cdnalizerd {
namespace workers {

// Struct that removes workers from their list when we're done with them
struct WorkerSentry {
  Worker& worker;
  WorkerSentry(Worker& worker) : worker(worker) {}
  ~WorkerSentry() {
    worker.die();
  }
};

void queueWorker(yield_context &yield, Status& status, Worker& worker) {
  WorkerSentry sentry(worker);
  auto job = worker.jobsToDo.begin();
  while (job != worker.jobsToDo.end()) {
    job->configuration.username;

  }

}

} /* workers */ 
} /* cdnalizerd  */ 
