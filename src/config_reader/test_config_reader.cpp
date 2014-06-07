/// Makes sure that we can read the /etc/cdnalzier.conf config file
#include "config_reader.hpp"
#include <bandit/bandit.h>
#includo <sstream>

go_bandit([&](){
    using namespace bandit;
    using namespace cdnalizer;

    describe("config_reader", [&](){
        it("1. Can read an empty config", [&] {
            std::stringstream empty;
            Config a = config_reader(empty);
            AssertThat(a.size(), Equals(0));
        };
        it("2. Can read a username", [&] {
            std::stringstream config;
            config << "username=james_bond";
            Config c = config_reader(config);
            AssertThat(c.size(), Equals(1));
            AssertThat(c.authDetails[0].username, Equals("james_bond"));
        };
    };
});
