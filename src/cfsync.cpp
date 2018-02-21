/// RSync emulator for to/from Rackspace cloud files

#include <boost/program_options.hpp>

#include <string>

namespace po = boost::program_options;

struct Options {
  std::string source;
  std::string dest;
};


po::options_description setupCommandlineOptions(int argc, char **argv) {
  po::options_description options("Options");
  options.add_options()("help", "Similar to rsync. Specify cloud files URLs "
                                "with 'cf://' at the beginning. Everything "
                                "else is considered a path")(
      "source", po::value<std::string>(), "Source path or cloud file URL")(
      "dest", po::value<std::string>(), "Destinanion path or cloud file URL");
  return options;
}

int main(int argc, char **argv) {
  po::options_description options(setupCommandlineOptions(argc, argv));
}

