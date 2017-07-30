#pragma once

#include <functional>
#include <ostream>

#include <RESTClient/rest.hpp>
#include <boost/filesystem.hpp>
#include <RESTClient/http/url.hpp>

#include "logging.hpp"

namespace cdnalizerd {

// These are here to give consistent naming for all users of the header
namespace fs = boost::filesystem;
using RESTClient::REST;
using RESTClient::http::URL;

struct Job {
  using Worker = std::function<void(RESTClient::REST&)>;
  static size_t nextId;
  const size_t id;
  const std::string name;
  const Worker go;
  Job(std::string name, Worker go)
      : id(nextId++), name(std::move(name)), go(go) {
    BOOST_LOG_TRIVIAL(debug) << "Job created: " << *this;
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
