/// CDNalizer Daemon
/// Watches a directory for changes and syncs it with a cloud files directory

#include <fstream>

#include "config_reader/config_reader.hpp"
#include "Status.hpp"

#include "processes/mainProcess.hpp"

using namespace cdnalizerd;

int main(int argc, char **argv) {
  // See if we have a --config_file option
  std::string config_file_name = "/etc/cdnalizerd.conf";
  if (argc == 2)
    config_file_name = argv[1];
  std::ifstream config_file(config_file_name);
  Config config = read_config(config_file);
  config_file.close();
  // Ensure we have a config
  if (!config) {
    using namespace std;
    cerr << "Usage: " << argv[0] << "[config_file_name]" << endl << endl
         << "Default config file is /etc/cdnalizerd.conf" << endl << endl
         << "Config file is read sequentially, so each option applies to the "
            "'path' entries below it." << endl << endl
         << "Each line is of the format: " << endl << "option=value"
         << "or" << endl << "/local/path /container/path" << endl << endl
         << "Paths can be \"quoted\"." << endl
         << "Available options are: " << endl
         << "username=string - rackspace username" << endl
         << "apikey=string - rackspace api key" << endl
         << "container=string - rackspace cloud files container name (where "
            "we're uploading to)" << endl
         << "region=string - SYD/DFW/ORD/IAD (no support for HKG/LON at this "
            "point)" << endl
         << "snet=on/off - User rackspace servicenet (on) or public internet "
            "(off) for uploading (defaults to off)" << endl
         << "move=on/off - move the files to the cloud (on) or just copy them "
            "(defaults to off)" << endl;
    return 1;
  }
  cdnalizerd::Status status;
  RESTClient::http::spawn([&config, &status](yield_context y) {
    cdnalizerd::processes::watchForFileChanges(y, status, config);
  });
  RESTClient::http::run();
  return 0;
}
