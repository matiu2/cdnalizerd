/// RSync emulator for to/from Rackspace cloud files

#include "commandLineOptions.hpp"
#include "cfsync.hpp"
#include "../config/config.hpp"
#include "../logging.hpp"
#include "../https.hpp"

#include <boost/program_options.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

#include <string>
#include <iostream>
#include <sstream>

namespace po = boost::program_options;
using namespace cdnalizerd;
namespace asio = boost::asio;

using namespace std::literals;

int main(int argc, char **argv) {
  Options options;
  if (!setupCommandlineOptions(argc, argv, options))
    return 1;

  // See if the source or destination is a cloud files repository (begins with
  // cf://)
  // +------------+--------------+-------------+
  // |            | remoteSource | localSource |
  // +------------+--------------+-------------+
  // | remoteDest | server_copy  |    upload   |
  // | localDest  |   download   |  local_copy |
  // +------------+--------------+-------------+
  JobType jobType;
  bool remoteSource = (boost::algorithm::starts_with(options.source, "cf://"s));
  bool remoteDest = (boost::algorithm::starts_with(options.dest, "cf://"s));
  if (remoteDest)
    jobType = remoteSource ? server_copy : upload;
  else
    jobType = remoteSource ? download : local_copy;

  // Remove the 'cf://' from strings
  if (remoteSource)
    options.source.erase(0, 5);
  if (remoteDest)
    options.dest.erase(0, 5);

  // We don't load a config file, so fill in the config from the command line options
  ConfigEntry entry;
  entry.local_dir = options.source;
  entry.remote_dir = options.dest;
  entry.snet = options.snet;
  options.region = options.region;
  options.apikey = options.apikey;
  options.username = options.username;

  asio::io_service ios;
  cdnalizerd::service(&ios);
  //asio::spawn(ios, std::bind(cfsync, std::placeholders::_1, jobType, std::ref(entry)));
  asio::spawn(ios, [jobType, &entry](asio::yield_context yield) {
    cfsync(yield, jobType, entry);
  });

  try {
    ios.run();
  } catch (boost::exception &e) {
    LOG_S(ERROR) << "Uncaught exception: "
                 << boost::diagnostic_information(e, true);
  } catch (std::exception &e) {
    LOG_S(ERROR) << "Uncaught exception (std::exception): "
                 << boost::diagnostic_information(e, true) << std::endl;
  } catch (...) {
    LOG_S(ERROR) << "Uncaught exception (unkown exception): "
                 << boost::current_exception_diagnostic_information(true)
                 << std::endl;
  }
  return 0;
}

