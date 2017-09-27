#include "Worker.hpp"

#include "logging.hpp"
#include "https.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/exception/exception.hpp>
#include <boost/exception/diagnostic_information.hpp> 

namespace cdnalizerd {

namespace err {

using jobName = boost::error_info<struct JobName, std::string>;
  
} /* err  */ 

// Removes workers from our list when we're done with them
struct OnDoneSentry {
  Worker& worker;
  OnDoneSentry(Worker &worker) : worker(worker) {}
  ~OnDoneSentry() {
    worker.onDone()();
  }
};

void doWork(Worker &worker, asio::yield_context yield) {
  try {
    // Find which worker wants this job
    OnDoneSentry onDoneSentry(worker);
    auto stateSentry = worker.setState(Working);
    // Connect
    LOG_SCOPE_FUNCTION(INFO);
    LOG_S(INFO) << "Worker " << &worker << " connecting to "
                << worker.url.host << std::endl;
    HTTPS conn(yield, worker.url.host);

    while (worker.hasMoreJobs()) {
      Job job = std::move(worker.getNextJob());
      LOG_S(INFO) << "Running job: " << job.id << " " << job.name << std::endl;
      try {
        job.go(conn, worker.token());
        LOG_S(INFO) << "Finished job: " << job.id << " " << job.name
                    << std::endl;
      } catch (boost::exception &e) {
        e << err::jobName(job.name);
        LOG_S(ERROR) << "Job failed: "
                     << boost::diagnostic_information(e, true);
      } catch (std::exception &e) {
        LOG_S(ERROR) << "Errored job (std::exception): " << job.id << " "
                     << job.name << ": "
                     << boost::diagnostic_information(e, true) << std::endl;
      } catch (...) {
        LOG_S(ERROR) << "Errored job (unkown exception): " << job.id << " "
                     << job.name
                     << boost::current_exception_diagnostic_information(true)
                     << std::endl;
      }
      if (!worker.hasMoreJobs()) {
        // If we have no more work to do, keep the connection open for some time
        stateSentry.updateState(Idle);
        boost::asio::deadline_timer idleTimer(service(),
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
  } catch (boost::exception &e) {
    LOG_S(ERROR) << "doWork failed: " << boost::diagnostic_information(e, true);
  } catch (std::exception &e) {
    LOG_S(ERROR) << "doWork failed (std::exception): "
                 << ": " << boost::diagnostic_information(e, true) << std::endl;
  } catch (...) {
    LOG_S(ERROR) << "doWork failed (unkown exception): "
                 << boost::current_exception_diagnostic_information(true)
                 << std::endl;
  }
}

void Worker::launch(std::function<void()> onDone) {
  assert(onDone);
  _onDone = onDone;
  _state = Ready;
  asio::spawn(service(),
              std::bind(doWork, std::ref(*this), std::placeholders::_1));
}

} /* cdnalizerd  */ 
