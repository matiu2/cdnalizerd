/// Makes sure that we can read the /etc/cdnalzier.conf config file
#include "config_reader.hpp"
#include "errors.hpp"
#include <bandit/bandit.h>
#include <sstream>
#include <string>

go_bandit([](){
    using namespace bandit;
    using namespace cdnalizerd;

    describe("config_reader", [&](){
        it("1. Can read an empty config", [&] {
            std::stringstream empty;
            Config a = read_config(empty);
        });
        it("2. Can read a username", [&] {
            std::stringstream config;
            config << "username=james_bond";
            Config c = read_config(config);
            AssertThat(c.usernames.size(), Equals(size_t(1)));
        });
        it("3. Can read an apikey", [&] {
            std::stringstream config;
            config << "apikey=james_bond";
            Config c = read_config(config);
            AssertThat(c.apikeys.size(), Equals(size_t(1)));
        });
        it("4. Can read a container", [&] {
            std::stringstream config;
            config << "container=james_bond";
            Config c = read_config(config);
            AssertThat(c.containers.size(), Equals(size_t(1)));
        });
        it("5. hates bad setting names", [&] {
            std::stringstream config;
            config << "xxx=james_bond";
            AssertThrows(ConfigError, read_config(config));
            std::string msg = LastException<ConfigError>().what();
            AssertThat(msg, Contains("Unkown setting"));
            AssertThat(msg, Contains("on line 1"));
        });
        it("6. knows that paths depend on containers and auth", [&] {
            std::stringstream config;
            config << "/path1 /path2";
            AssertThrows(ConfigError, read_config(config));
            std::string msg = LastException<ConfigError>().what();
            AssertThat(msg, Equals("Need to have a valid username, apikey and container before adding a path pair"));
        });

        std::string starter{R"(
            username=hello
            apikey=1234
            container=publish
        )"};

        it("7. Can read a simple path pair", [&] {
            std::stringstream config;
            config << starter;
            config << "/source/path /destination/path" << std::endl;
            Config c = read_config(config);
            EntryData e = c.getEntryByPath("/source/path");
            AssertThat(e.username, Equals("hello"));
            AssertThat(e.apikey, Equals("1234"));
            AssertThat(e.container, Equals("publish"));
            AssertThat(e.local_dir, Equals("/source/path"));
            AssertThat(e.remote_dir, Equals("/destination/path"));
        });
    });
});

int main(int argc, char** argv) {
    return bandit::run(argc, argv);
}
