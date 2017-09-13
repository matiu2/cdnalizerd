#pragma once

#include "Rackspace.hpp"
#include "Job.hpp"

#include <queue>
#include <memory>
#include <functional>

#include <boost/asio.hpp>

#include <LUrlParser.h>

namespace cdnalizerd {

enum WorkerState { Raw, Ready, Working, Idle, Dead };
namespace asio = boost::asio;

class StateSentry {
private:
  WorkerState& state;
  bool aborted;
public:
  StateSentry(WorkerState &state) : state(state), aborted(false) {}
  void abort() {
    aborted = true;
  }
  ~StateSentry() {
    if (!aborted)
      state = Dead;
  }
  void updateState(WorkerState newState) { state = newState; }
};

using URL = LUrlParser::clParseURL;

class Worker {
private:
  // Info for making connections
  const Rackspace& rs;
  WorkerState _state;
  // Mechanism to stop working when we have no new jobs
  std::function<void()> _onDone;
  // A worker will tend to stick to its same URL so it can reuse the connection
  std::queue<Job> _queue;
  void doActualWork();

public:
  // Consstructor
  Worker(const Rackspace &rs, URL url)
      : rs(rs), _state(Raw), url(std::move(url)) {}
  Worker(const Worker&) = delete;
  Worker(Worker&&) = default;
  void launch(std::function<void()> onDone);
  WorkerState state() const { return _state; }
  bool idle() const { return (_state == Ready) || (_state == Idle); }
  size_t queueSize() const { return _queue.size(); }
  void addJob(Job&& job) {
    _queue.push(std::move(job));
  }
  const std::function<void()> &onDone() const {
    assert(_onDone);
    return _onDone;
  }
  Job getNextJob() {
    assert(_queue.size() > 0);
    Job result = std::move(_queue.front());
    _queue.pop();
    return result;
  }
  bool hasMoreJobs() const { return _queue.size() > 0; }
  const std::string &token() const { return rs.token(); }
  StateSentry setState(WorkerState newState) {
    _state = newState;
    return StateSentry(_state);
  }
  const URL url;
};


} /* cdnalizerd */ 

