#include "mainProcess.hpp"

#include "../inotify.hpp"

#include "queueWorker.hpp"
#include "../AccountCache.hpp"
#include "../operations/inotifyToJob.hpp"

namespace cdnalizerd {
namespace processes {

constexpr int maskToFollow =
    IN_CREATE | IN_CLOSE_WRITE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO;

void readConfig(yield_context &yield, Status &status, const Config &config,
                inotify::Instance &inotify) {
  for (const ConfigEntry &entry : config.entries()) {
    // Starts watching a directory
    inotify::Watch &watch =
        inotify.addWatch(entry.local_dir.c_str(), maskToFollow);
    status.watchToConfig[watch.handle()] = entry;
    walkDir(entry.local_dir.c_str(), [&](const char *path) {
      if (!inotify.alreadyWatching(path))
        inotify.addWatch(path, maskToFollow);
    });
  }
}

void queueJob(Job &&job, Status &status, const Config &config, AccountCache& accounts) {
  // Find or create the worker for this job
  std::list<Worker> workers =
      status.workers[{job.configuration.username, job.configuration.region}];
  // We like to have up to 3 workers per connection
  if (workers.size() < 3) {
    // Create a new worker
    Rackspace& rs = accounts[job.configuration.username];
    Worker worker(workers, rs);
    worker.jobsToDo.emplace_back(std::move(job));
    assert(rs.status() == Rackspace::Ready);
    worker.rs = rs;
    workers.emplace_back(std::move(worker));
    workers.back().me = --workers.end();
    RESTClient::http::spawn(
        [&](yield_context y) { queueWorker(y, status, workers.back()); });
  } else {
    // Find which worker has the least jobs
    auto worker = std::min_element(
        workers.begin(), workers.end(), [](const Worker &a, const Worker &b) {
          return a.jobsToDo.size() < b.jobsToDo.size();
        });
    worker->jobsToDo.emplace_back(std::move(job));
  }
}

void watchForFileChanges(yield_context &yield, Status &status,
                         const Config &config) {
  // Setup
  inotify::Instance inotify(yield);
  readConfig(yield, status, config, inotify);
  AccountCache accounts;

  // Spawn some workers to fill in the account info (token and urls)
  boost::asio::deadline_timer waitForLogins(*RESTClient::tcpip::getService(),
                                            boost::posix_time::minutes(10));
  int loginWorkers = 2;
  for (int i = 0; i != loginWorkers; ++i) {
    RESTClient::http::spawn([&](yield_context y) {
      fillAccountCache(y, config, accounts, [&loginWorkers, &waitForLogins]() {
        --loginWorkers;
        if (loginWorkers == 0)
          waitForLogins.cancel();
      });
    });
  }
  // Wait for all the logins to finish
	waitForLogins.async_wait(yield);

  assert("Logins to Rackspace timed out");

  std::list<Job> jobsToDo;

  while (true) {
    inotify::Event event = inotify.waitForEvent();
    // If it's a move event, find its pair
    if (event.cookie) {
      auto found = status.cookies.find(event.cookie);
      if (found != status.cookies.end()) {
        if (event.wasMovedTo())
          std::swap(event, found->second);
        if (event.wasMovedFrom())
          event.destination.reset(new inotify::Event(std::move(found->second)));
        /// TODO: Log a runtime error here, only move events should have cookies
        status.cookies.erase(found);
      } else {
        // The event's partner hasn't been found yet.
        // Store it in cookies; start another worker that'll wait 1 second, then
        // launch it as a delete if it's still there
        // TODO: Launch the cleanup worker
        continue; // Wait for its partner to appear
      }
    }
    // Now we have a whole event...
    if (event.wasCreated()) {
      // ... if a directory was created, add a watch to it
      std::string path = event.path();
      if (isDir(path.c_str()))
        inotify.addWatch(path.c_str(), maskToFollow);
    } else {
      // Get the next job that needs doing
      Job job{inotifyEventToJob(status, std::move(event))};
      // Put it in the right queue
      queueJob(std::move(job), status, config, accounts);
    }
  }
}
    

} /* processes  */ 
} /* cdnalizerd  */ 
