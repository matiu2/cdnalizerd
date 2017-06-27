/// Makes sure that we can read the /etc/cdnalzier.conf config file
#include "config_reader.hpp"
#include "errors.hpp"
#include <bandit/bandit.h>
#include <sstream>
#include <string>

go_bandit([]() {
  using namespace bandit;
  using namespace cdnalizerd;

  using snowhouse::Equals;
  using snowhouse::LastException;
  using snowhouse::Contains;

  describe("config_reader", [&]() {
    it("1. Can read an empty config", [&] {
      std::stringstream empty;
      Config a = read_config(empty);
    });
    it("2. Can read a username", [&] {
      std::stringstream config;
      config << "username=james_bond";
      Config c = read_config(config);
      AssertThat(c.entries().size(), Equals(size_t(1)));
      AssertThat(*c.entries().front().username, Equals("james_bond"));
      
    });
    it("3. Can read an apikey", [&] {
      std::stringstream config;
      config << "apikey=james_bond";
      Config c = read_config(config);
      AssertThat(c.entries().size(), Equals(size_t(1)));
      AssertThat(*c.entries().front().apikey, Equals("james_bond"));
    });
    it("4. Can read a container", [&] {
      std::stringstream config;
      config << "container=james_bond";
      Config c = read_config(config);
      AssertThat(c.entries().size(), Equals(size_t(1)));
    });
    it("5. hates bad setting names", [&] {
      std::stringstream config;
      config << "xxx=james_bond";
      AssertThrows(ConfigError, read_config(config));
      std::string msg = LastException<ConfigError>().what();
      AssertThat(msg, Contains("Unkown setting"));
      AssertThat(msg, Contains("on line 1"));
    });
    it("6. knows that paths depend on containers, regions and auth", [&] {
      std::stringstream config;
      config << "/path1 /path2";
      AssertThrows(ConfigError, read_config(config));
      std::string msg = LastException<ConfigError>().what();
      AssertThat(msg, Equals("Need to have a valid username, apikey, region "
                             "and container before adding a path pair"));
    });

    std::string starter{R"(
            username=hello
            apikey=1234
            container=publish
            region=SYD
        )"};

    it("7. Can read a simple path pair", [&] {
      std::stringstream config;
      config << starter;
      config << "/source/path /destination/path" << std::endl;
      Config c = read_config(config);
      const ConfigEntry &e = c.getEntryByPath("/source/path");
      AssertThat(*e.username, Equals("hello"));
      AssertThat(*e.apikey, Equals("1234"));
      AssertThat(*e.container, Equals("publish"));
      AssertThat(*e.region, Equals("SYD"));
      AssertThat(*e.local_dir, Equals("/source/path"));
      AssertThat(*e.remote_dir, Equals("/destination/path"));
    });

    std::string service_net{R"(
            # snet defaults to off
            username=hello
            apikey=1234
            container=publish
            region=SYD
            /source/path /destination/path
        )"};

    it("8. Knows about service_net", [&] {
      std::stringstream config;
      config << starter << std::endl << "/source/path /destination/path"
             << std::endl << "# Now we'll try with snet" << std::endl
             << "snet=true" << std::endl << "/source/path2 /destination/path2"
             << std::endl;
      Config c = read_config(config);
      const ConfigEntry &e = c.getEntryByPath("/source/path");
      AssertThat(*e.username, Equals("hello"));
      AssertThat(*e.apikey, Equals("1234"));
      AssertThat(*e.container, Equals("publish"));
      AssertThat(*e.region, Equals("SYD"));
      AssertThat(*e.local_dir, Equals("/source/path"));
      AssertThat(*e.remote_dir, Equals("/destination/path"));
      AssertThat(e.snet, Equals(false));
      const ConfigEntry &e2 = c.getEntryByPath("/source/path2");
      AssertThat(*e2.username, Equals("hello"));
      AssertThat(*e2.apikey, Equals("1234"));
      AssertThat(*e2.container, Equals("publish"));
      AssertThat(*e2.region, Equals("SYD"));
      AssertThat(*e2.local_dir, Equals("/source/path2"));
      AssertThat(*e2.remote_dir, Equals("/destination/path2"));
      AssertThat(e2.snet, Equals(true));
    });
  });
});

int main(int argc, char **argv) { return bandit::run(argc, argv); }
