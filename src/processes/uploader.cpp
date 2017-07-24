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

using RESTClient::http::URL;
using RESTClient::http::Response;
using RESTClient::REST;

// Removes workers from our list when we're done with them
struct WorkerSentry {
  Worker& worker;
  WorkerSentry(Worker& worker) : worker(worker) {}
  ~WorkerSentry() {
    worker.onDone()();
  }
};

void handleJob(Job&& job, REST& conn) {
  URL source(job.source);
  URL dest(job.dest);
  switch (job.operation) {
    case Upload: {
      std::ifstream file(job.source);
      conn.put(dest.path_part()).body(file).go();
      break;
    }
    case Move: {
      {
        std::ifstream file(job.source);
        conn.put(dest.path_part()).body(file).go();
      }
      // Delete the source file
      remove(job.dest.c_str());
      break;
    }
    case SCopy: {
      if (source.hostname != dest.hostname) {
        // TODO: support a download, then upload copy ?
        throw std::runtime_error("We don't support server side copy between two different hosts");
      }
      conn.put(dest.path_part()).add_header("X-Copy-From", source.path_part()).go();
      break;
    }
    case SDelete: {
      conn.delet(source.path_part()).go();
    }
  };
}

void uploader(yield_context yield, Worker &worker) {
  // Find which worker wants this job
  WorkerSentry sentry(worker);
  // Grab the first job in the queue
  Job job(worker.getNextJob());
  // Connect
  RESTClient::http::URL url(job.workerURL());
  REST conn(yield, url.host_part(), {{"Content-type", "application/json"},
                                     {"X-Auth-Token", worker.token()}});
  auto stateSentry = worker.setState(Working);
  handleJob(std::move(job), conn);
  do {
    job = {worker.getNextJob()};
    handleJob(std::move(job), conn);
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
