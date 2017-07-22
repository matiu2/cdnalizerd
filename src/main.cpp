/// CDNalizer Daemon
/// Watches a directory for changes and syncs it with a cloud files directory

#include <fstream>

#include "globals.hpp"
#include "config_reader/config_reader.hpp"
#include "config_reader/config_writer.hpp"
#include "processes/mainProcess.hpp"
#include "logging.hpp"

#include <boost/program_options.hpp>

using namespace cdnalizerd;

namespace po = boost::program_options;

po::variables_map parseCommandline(int argc, char** argv) {
  po::options_description desc("Allowed options");
  desc.add_options()("help", "Show help message")(
      "config", po::value<std::string>()->default_value("/etc/cdnalizerd.conf"),
      "The configuration file or directory "
      "name. Accepts files with extensions "
      ".info or .ini")("create-sample",
                       "Creates an empty sample config file at 'config' "
                       "filename, and does nothing else");
  po::variables_map result;
  po::store(po::parse_command_line(argc, argv, desc), result);
  result.notify();
  if (result.count("help")) {
    using namespace std;
    cout << desc << std::endl;
  }
  return result;
}

int main(int argc, char **argv) {
  // See if we have a --config_file option
  po::variables_map options(parseCommandline(argc, argv));
  std::string config_file_name = options["config"].as<std::string>();
  if (options.count("create-sample")) {
    cdnalizerd::write_sample_config(config_file_name);
    std::cerr << "Sample config file written to " << config_file_name << std::endl;
  } else if (options.count("help")) {
    // The options were already prentid in 'parseCommandline'. Just exit
    return -1;
  } else {
    BOOST_LOG_TRIVIAL(info) << "Reading config from " << config_file_name;
    config = read_config(config_file_name);
    RESTClient::http::spawn(cdnalizerd::processes::watchForFileChanges);
    RESTClient::http::run();
    return 0;
  }
}
