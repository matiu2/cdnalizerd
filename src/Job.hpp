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
  using Work = std::function<void(HTTPS&)>;
  static size_t nextId;
  const size_t id;
  const std::string name;
  const Work go;
  Job(std::string name, Work go)
      : id(nextId++), name(std::move(name)), go(go) {
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
