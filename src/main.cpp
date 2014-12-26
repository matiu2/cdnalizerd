/// CDNalizer Daemon
/// Watches a directory for changes and syncs it with a cloud files directory

#include <curlpp11.hpp>
#include <fstream>

#include "config_reader/config_reader.hpp"
#include "Watcher.hpp"

#include "curlpp11.hpp"

using namespace cdnalizerd;

int main(int argc, char** argv)
{
  curl::GlobalSentry curl;
  // See if we have a --config_file option
  std::string config_file_name = "/etc/cdnalizerd.conf";
  if (argc == 3)
    if (std::string(argv[2]) == "--config_file")
      config_file_name = argv[3];
  std::ifstream config_file(config_file_name);
  Config config = read_config(config_file);
  config_file.close();
  // Start watching dirs
  Watcher watcher(config);
  return 0;
}
