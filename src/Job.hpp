#pragma once

#include "logging.hpp"
#include "https.hpp"

#include <functional>
#include <iostream>
#include <ostream>

#include <boost/filesystem.hpp>

namespace cdnalizerd {

// These are here to give consistent naming for all users of the header
namespace fs = boost::filesystem;

struct Job;

inline std::ostream &operator<<(std::ostream &out, const Job& job);
 
struct Job {
  using Work = std::function<void(HTTPS&, const std::string&)>;
  static size_t nextId;
  const size_t id;
  const std::string name;
  const Work work;
  void go(HTTPS& conn, const std::string& token)  {
    LOG_S(3) << "job: start " << id << " - " << name;
    try {
      work(conn, token);
    } catch(...) {
      LOG_S(3) << " job: failed " << id << " - " << name;
      throw;
    }
    LOG_S(3) << " job: done " << id << " - " << name;
  }
  Job(std::string name, Work work)
      : id(nextId++), name(std::move(name)), work(work) {
    LOG_S(5) << "Job created: " << *this << std::endl;
  }
  Job(Job&& other) = default;
   // Can't copy them because then you'd have two with the same ID
  Job(const Job& other) = delete;
};

inline std::ostream &operator<<(std::ostream &out, const Job& job) {
  out << "Job id(" << job.id << ") \"" << job.name << "\"";
  return out;
}

}
