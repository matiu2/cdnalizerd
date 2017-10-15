/// CDNalizer Daemon
/// Watches a directory for changes and syncs it with a cloud files directory

#include <iostream>
#include <fstream>

#include "config_reader/config_reader.hpp"
#include "config_reader/config_writer.hpp"
#include "processes/mainProcess.hpp"
#include "processes/list.hpp"
#include "processes/login.hpp"
#include "logging.hpp"
#include "https.hpp"
#include "exception_tags.hpp"

#include <boost/program_options.hpp>
#include <boost/log/trivial.hpp>

using namespace cdnalizerd;

namespace po = boost::program_options;

int main(int argc, char **argv) {
  asio::io_service ios;
  cdnalizerd::service(&ios);
  // See if we have a --config_file option
  po::options_description desc("Allowed options");
  desc.add_options()("help", "Show help message")(
      "config", po::value<std::string>()->default_value("/etc/cdnalizerd.conf"),
      "The configuration file or directory "
      "name. Accepts files with extensions "
      ".info or .ini")("create-sample",
                       "Creates an empty sample config file at 'config' "
                       "filename, and does nothing else")("go",
                                                          "Start running...")(
      "list", "List all the containers from the config to standard out")(
      "list-detailed",
      "List all the containers from the config to standard "
      "out, including md5sum, modification date (in UTC), "
      "content-type, size")("log-verbosity", po::value<int>()->default_value(0),
                            "-9 to 9 - FATAL=-3, INFO=0, DEBUG=5, TRACE=9");
  po::variables_map options;
  po::store(po::parse_command_line(argc, argv, desc), options);
  options.notify();
  
  assert(options.count("log-verbosity"));
  int verbosity = options["log-verbosity"].as<int>();
  if ((verbosity < -9) || (verbosity > 9)) {
    init_logging(9);
    LOG_S(FATAL) << "--log-verbosity must be between -9 and 9";
    return -1;
  }
  init_logging(verbosity);

  // Handle the options
  std::string config_file_name = options["config"].as<std::string>();
  if (options.count("create-sample")) {
    try {
      cdnalizerd::write_sample_config(config_file_name);
      LOG_S(INFO) << "Sample config file written to " << config_file_name
                  << std::endl;
    } catch (std::exception& e) {
      LOG_S(WARNING) << e.what();
      return -1;
    }
    return 0;
  }
  if (options.count("list") || options.count("list-detailed") ||
      options.count("go")) {
    LOG_S(INFO) << "Reading config from " << config_file_name << std::endl;
    Config config = read_config(config_file_name);
    if (options.count("list"))
      asio::spawn(ios, [&config](yield_context yield) {
        AccountCache accounts;
        login(yield, accounts, config);
        cdnalizerd::processes::listContainers(std::move(yield), accounts, config);
      });
    else if (options.count("list-detailed"))
      asio::spawn(ios, [&config](yield_context yield) {
        AccountCache accounts;
        login(yield, accounts, config);
        cdnalizerd::processes::JSONListContainers(std::move(yield), accounts, config);
      });
    else if (options.count("go"))
      asio::spawn(ios, [&config](yield_context yield) {
        cdnalizerd::processes::watchForFileChanges(std::move(yield), config);
      });
    try {
      ios.run();
    } catch (boost::exception &e) {
      LOG_S(ERROR) << "Uncaught exception: " << boost::diagnostic_information(e, true);
    } catch (std::exception &e) {
      LOG_S(ERROR) << "Uncaught exception (std::exception): "
                   << boost::diagnostic_information(e, true) << std::endl;
    } catch (...) {
      LOG_S(ERROR) << "Uncaught exception (unkown exception): "
                   << boost::current_exception_diagnostic_information(true)
                   << std::endl;
    }
    return 0;
  } else {
    using namespace std;
    cout << desc << std::endl;
    return -1;
  }
}
