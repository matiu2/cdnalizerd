#include "uploader.hpp"

#include "../Rackspace.hpp"
#include "../Status.hpp"
#include "../config_reader/config.hpp"
#include "../globals.hpp"
#include "../Worker.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <ifstream>

#include <RESTClient/rest.hpp>

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

void handleJob(const Job& job, REST& conn) {
  switch (job.operation) {
    case Upload: {
      std::ifstream file(job.source);
      conn.put(job.dest).body(file).go();
    }
  };
}

void uploader(yield_context yield, Worker& worker) {
  // Find which worker wants this job
  WorkerSentry sentry(worker);
  // Grab the first job in the queue
  auto job(worker.getNextJob());
  // Grab the url that we're to connect to
  const std::string& url = job.workerURL();
  // Connect
  REST connection(yield, job.workerURL(), {{"Content-type", "application/json"},
                                           {"X-Auth-Token", worker.token()}});
  handleJob(job);
  while (worker.hasMoreJobs()) {
    Job job{worker.getNextJob()};
  }
}

} /* processes */ 
} /* cdnalizerd  */ 
