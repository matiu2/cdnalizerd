#include "commandLineOptions.hpp"

#include "../logging.hpp"

#include <boost/program_options.hpp>

#include <iostream>

namespace po = boost::program_options;

bool setupCommandlineOptions(int argc, char **argv, Options &result) {
  po::options_description options("Options");
  options.add_options()("help", "Similar to rsync. Specify cloud files URLs "
                                "with 'cf://' at the beginning. Everything "
                                "else is considered a path")(
      "source", po::value<std::string>(&result.source),
      "Source path or cloud file URL (cf://)")(
      "dest", po::value<std::string>(&result.dest),
      "Destinanion path or cloud file URL (cf://)")(
      "username", po::value<std::string>(&result.username),
      "Rackspace username")("apikey", po::value<std::string>(&result.apikey),
                            "Rackspace api key")(
      "dc", po::value<std::string>(&result.region),
      "Rackspace datacenter DFW / IAD / LON / ORD / SYD")(
      "snet", po::value<bool>(&result.snet),
      "Use service net (intertal DC network)")(
      "log-verbosity", po::value<int>(&result.log_verbosity)->default_value(0),
      "-9 to 9 - FATAL=-3, INFO=0, DEBUG=5, TRACE=9");
  po::positional_options_description p;
  p.add("source", 1);
  p.add("dest", 1);
  po::variables_map vm;

  struct Error {};
  std::stringstream error_message;

  // Apply all the options if you can
  try {
    try {
      po::store(po::command_line_parser(argc, argv)
                    .options(options)
                    .positional(p)
                    .run(),
                vm);
    } catch (boost::program_options::error &e) {
      error_message << "Error parsing options: " << e.what() << "\n";
      throw Error();
    }
    po::notify(vm);
    if (result.source.empty()) {
      error_message << "No source file/url provided";
      throw Error();
    }
    if (result.dest.empty()) {
      error_message << "No destination file/url provided";
      throw Error();
    }
    // Parse the options
    if (vm.count("help")) {
      options.print(std::cout, 80);
      return false;
    }
  } catch (Error) {
    std::cerr << error_message.str() << "\n";
    options.print(std::cerr, 80);
    return false;
  }

  // Process logging
  if ((result.log_verbosity < -9) || (result.log_verbosity > 9)) {
    cdnalizerd::init_logging(9);
    LOG_S(FATAL) << "--log-verbosity must be between -9 and 9";
    return -1;
  }

  cdnalizerd::init_logging(result.log_verbosity);
  return true;
}
