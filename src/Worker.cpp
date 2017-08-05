#include "Worker.hpp"

#include <RESTClient/rest.hpp>
#include <RESTClient/tcpip/interface.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio/deadline_timer.hpp>

#include "logging.hpp"

namespace cdnalizerd {

using RESTClient::http::URL;
using RESTClient::http::Response;
using RESTClient::REST;

// Removes workers from our list when we're done with them
struct OnDoneSentry {
  Worker& worker;
  OnDoneSentry(Worker &worker) : worker(worker) {}
  ~OnDoneSentry() {
    worker.onDone()();
  }
};

void doWork(Worker &worker, yield_context yield) {
  // Find which worker wants this job
  OnDoneSentry onDoneSentry(worker);
  auto stateSentry = worker.setState(Working);
  // Connect
  REST conn(
      yield, worker.url.host_part(),
      {{"Content-type", "application/json"}, {"X-Auth-Token", worker.token()}});
  while (worker.hasMoreJobs()) {
    Job job = std::move(worker.getNextJob());
    BOOST_LOG_TRIVIAL(info) << "Running job: " << job.id << " " << job.name;
    try {
      job.go(conn);
    } catch ( std::exception& e ) {
      BOOST_LOG_TRIVIAL(error) << "Errored job: " << job.id << " " << job.name << ": " << e.what();
    } catch (...) {
      BOOST_LOG_TRIVIAL(error) << "Errored job: " << job.id << " " << job.name << ": UNKNOWN ERROR";
    }
    BOOST_LOG_TRIVIAL(info) << "Finished job: " << job.id << " " << job.name;
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
}

void Worker::launch(std::function<void()> onDone) {
  assert(onDone);
  _onDone = onDone;
  _state = Ready;
  RESTClient::http::spawn(std::bind(doWork, std::ref(*this), std::placeholders::_1));
}

} /* cdnalizerd  */ 
