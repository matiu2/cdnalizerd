#pragma once

#include <map>
#include <list>

#include "Worker.hpp"

namespace cdnalizerd {

constexpr size_t MAX_WORKERS_PER_URL = 3;
  
class WorkerManager {
private:
  /// Maps a url to a list of workers with open connections to that URL
  std::map<std::string, std::list<Worker>> workers;
public:
  /// Returns an iterator to the worker with the least load; creates a worker if
  /// necessary
  std::list<Worker>::iterator getWorker(const std::string &url, const Rackspace& rs) {
    std::list<Worker> &list(workers[url]);
    if (list.size() < MAX_WORKERS_PER_URL) {
      list.push_front(Worker(rs, url));
      auto result = list.begin();
      result->launch([result, &list, this]() { list.erase(result); });
      return result;
    } else {
      // Find an idle worker, or the worker with the least jobs
      auto leastBusy = list.begin();
      for (auto result = list.begin(); result != list.end(); ++result) {
        if (result->idle())
          return result;
        if (result->queueSize() < leastBusy->queueSize())
          leastBusy = result;
      }
      return leastBusy;
    }
  }
};

} /* cdnalizerd  */ 
