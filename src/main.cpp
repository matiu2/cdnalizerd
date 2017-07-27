/// CDNalizer Daemon
/// Watches a directory for changes and syncs it with a cloud files directory

#include <iostream>
#include <fstream>

#include "config_reader/config_reader.hpp"
#include "config_reader/config_writer.hpp"
#include "processes/mainProcess.hpp"
#include "processes/list.hpp"
#include "logging.hpp"

#include <boost/program_options.hpp>
#include <RESTClient/http/interface.hpp>

using namespace cdnalizerd;

namespace po = boost::program_options;

int main(int argc, char **argv) {
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
      "list-detailed", "List all the containers from the config to standard "
                       "out, including md5sum, modification date (in UTC), "
                       "content-type, size");
  po::variables_map options;
  po::store(po::parse_command_line(argc, argv, desc), options);
  options.notify();

  // Handle the options
  std::string config_file_name = options["config"].as<std::string>();
  if (options.count("create-sample")) {
    cdnalizerd::write_sample_config(config_file_name);
    BOOST_LOG_TRIVIAL(info) << "Sample config file written to "
                            << config_file_name;
    return 0;
  }
  if (options.count("list") || options.count("list-detailed") ||
      options.count("go")) {
    BOOST_LOG_TRIVIAL(info) << "Reading config from " << config_file_name;
    Config config = read_config(config_file_name);
    if (options.count("list"))
      RESTClient::http::spawn([&config](yield_context yield) {
        cdnalizerd::processes::listContainers(std::move(yield), config);
      });
    else if (options.count("list-detailed"))
      RESTClient::http::spawn([&config](yield_context yield) {
        cdnalizerd::processes::JSONListContainers(std::move(yield), config);
      });
    else if (options.count("go"))
      RESTClient::http::spawn([&config](yield_context yield) {
        cdnalizerd::processes::watchForFileChanges(std::move(yield), config);
      });
    RESTClient::http::run();
    return 0;
  } else {
    using namespace std;
    cout << desc << std::endl;
    return -1;
  }
}
