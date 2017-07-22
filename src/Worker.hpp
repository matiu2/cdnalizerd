#pragma once

#include <queue>
#include <functional>

#include "Rackspace.hpp"
#include "Job.hpp"

namespace cdnalizerd {

enum WorkerState { Raw, Ready, Working, Idle, Dead };

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

class Worker {
private:
  // Info for making connections
  Rackspace& rs;
  WorkerState _state;
  // Mechanism to stop working when we have no new jobs
  std::function<void()> _onDone;
  // A worker will tend to stick to its same URL so it can reuse the connection
  std::queue<Job> _queue;
  void doActualWork();

public:
  // Consstructor
  Worker(Rackspace& rs) : rs(rs), _state(Raw) {}
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
    auto result = std::move(_queue.front());
    _queue.pop();
    return result;
  }
  bool hasMoreJobs() const { return _queue.size() > 0; }
  const std::string &token() const { return rs.token(); }
  StateSentry setState(WorkerState newState) {
    _state = newState;
    return StateSentry(_state);
  }
};


} /* cdnalizerd */ 
