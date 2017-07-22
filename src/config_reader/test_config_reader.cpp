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
    it("1. Can read an ini file", [&] {
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

    it("2. Knows about service_net and can read a .info file ", [&]() {
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

    it("3. Can read a directory", [&]() {
      namespace fs = boost::filesystem;
      fs::path dir("tmp");
      if (!fs::exists(dir))
        fs::create_directory(dir);
      std::ofstream config("tmp/tmp.ini");
      config << "username=james_bond\n";
      config << "apikey=key key key\n";
      config << "container=    the.box    \n";
      config << "region=ORD\n";
      config << "endpoint = main\n";
      config << "local_dir = /images\n";
      config << "remote_dir = /remote_images\n";
      config.close();
      std::ofstream config2("tmp/tmp.info");
      config2 << "username world\n"
             << "apikey 5678\n"
             << "container movies\n"
             << "region DFW\n"
             << "local_dir /amazing/videos\n"
             << "remote_dir /destination/videos\n";
      config2.close();
      Config c = read_config(dir.string());
      AssertThat(c.entries().size(), Equals(2u));
      const ConfigEntry &e = c.getEntryByPath("/images");
      AssertThat(*e.username, Equals("james_bond"));
      AssertThat(*e.apikey, Equals("key key key"));
      AssertThat(*e.container, Equals("the.box"));
      AssertThat(*e.region, Equals("ORD"));
      AssertThat(e.local_dir, Equals("/images"));
      AssertThat(e.remote_dir, Equals("/remote_images"));
      AssertThat(e.snet, Equals(false));
      const ConfigEntry e2 = c.getEntryByPath("/amazing/videos");
      AssertThat(*e2.username, Equals("world"));
      AssertThat(*e2.apikey, Equals("5678"));
      AssertThat(*e2.container, Equals("movies"));
      AssertThat(*e2.region, Equals("DFW"));
      AssertThat(e2.local_dir, Equals("/amazing/videos"));
      AssertThat(e2.remote_dir, Equals("/destination/videos"));
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
