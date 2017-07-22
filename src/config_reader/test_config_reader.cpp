/// Makes sure that we can read the /etc/cdnalzier.conf config file
#include "config_reader.hpp"
#include "errors.hpp"
#include <bandit/bandit.h>
#include <boost/filesystem.hpp>
#include <fstream>
#include <sstream>
#include <string>

go_bandit([]() {
  using namespace bandit;
  using namespace cdnalizerd;

  using snowhouse::Equals;
  using snowhouse::LastException;
  using snowhouse::Contains;

  describe("config_reader", [&]() {
    it("2. Can read an ini file", [&] {
      std::ofstream config("tmp.ini");
      config << "username=james_bond\n";
      config << "apikey=key key key\n";
      config << "container=    the.box    \n";
      config << "region=ORD\n";
      config << "endpoint = main\n";
      config << "local_dir = /images\n";
      config << "remote_dir = /remote_images\n";
      config.close();
      Config c = read_config("tmp.ini");
      // One entry per path line
      AssertThat(c.entries().size(), Equals(1u));
      const auto& entry = c.entries().front();
      AssertThat(*entry.username, Equals("james_bond"));
      AssertThat(*entry.apikey, Equals("key key key"));
      AssertThat(*entry.container, Equals("the.box"));
      AssertThat(*entry.region, Equals("ORD"));
      AssertThat(entry.local_dir, Equals("/images"));
      AssertThat(entry.remote_dir, Equals("/remote_images"));
      AssertThat(entry.snet, Equals(false));
    });

    it("8. Knows about service_net and can read a .info file ", [&]() {
      std::ofstream config("tmp.info");
      config << "username hello\n"
             << "apikey 1234\n"
             << "container publish\n"
             << "region SYD\n"
             << "local_dir /source/path\n"
             << "remote_dir /destination/path\n";
      config.close();
      Config c = read_config("tmp.info");
      const ConfigEntry &e = c.getEntryByPath("/source/path");
      AssertThat(*e.username, Equals("hello"));
      AssertThat(*e.apikey, Equals("1234"));
      AssertThat(*e.container, Equals("publish"));
      AssertThat(*e.region, Equals("SYD"));
      AssertThat(e.local_dir, Equals("/source/path"));
      AssertThat(e.remote_dir, Equals("/destination/path"));
      AssertThat(e.snet, Equals(false));
    });
  });
});

int main(int argc, char **argv) {
  namespace fs = boost::filesystem;
  fs::path dir("tmp.info");
  if (fs::is_directory(dir)) {
    std::cout << "Dir: " << std::endl;
    fs::directory_iterator end;
    for (auto f = fs::directory_iterator(dir); f != end; ++f)
      std::cout << f->path().native() << std::endl;
  } else {
    // This isn't a directory name, it's a filename
    std::cout << "File: " << dir.native() << std::endl;
  }
  return bandit::run(argc, argv);
}
