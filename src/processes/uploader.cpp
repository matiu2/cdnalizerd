#include "uploader.hpp"

#include "../Rackspace.hpp"
#include "../config_reader/config.hpp"
#include "../Worker.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <fstream>
#include <stdio.h>

#include <RESTClient/rest.hpp>
#include <RESTClient/tcpip/interface.hpp>

namespace cdnalizerd {
namespace processes {

using RESTClient::REST;

// Removes workers from our list when we're done with them
struct WorkerSentry {
  Worker& worker;
  WorkerSentry(Worker& worker) : worker(worker) {}
  ~WorkerSentry() {
    worker.onDone()();
  }
};

void uploader(yield_context yield, Worker &worker) {
  // Find which worker wants this job
  WorkerSentry sentry(worker);
  // Connect
  REST conn(yield, worker.url.host_part(), {{"Content-type", "application/json"},
                                     {"X-Auth-Token", worker.token()}});
  auto stateSentry = worker.setState(Working);
  do {
    Job job(worker.getNextJob());
    job.go(conn);
    if (!worker.hasMoreJobs()) {
      // If we have no more work to do, keep the connection open for some time 
      stateSentry.updateState(Idle);
      boost::asio::deadline_timer idleTimer(*RESTClient::tcpip::getService(),
                                            boost::posix_time::seconds(20));
      idleTimer.async_wait(yield);
      if (worker.hasMoreJobs())
        stateSentry.updateState(Working);
      else {
        stateSentry.updateState(Dead);
        break;
      }
    }
  }

          while (worker.hasMoreJobs());


}
} /* processes */
} /* cdnalizerd  */
