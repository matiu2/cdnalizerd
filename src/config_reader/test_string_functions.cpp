/// Tests string functions

#include "string_functions.hpp"

#include <bandit/bandit.h>
#include <sstream>

go_bandit([](){
    using namespace bandit;
    using namespace string_fun;
    using std::string;


    describe("trim", [&](){
        it("1. Doesn't touch an empty string", [&] {
            string data="Already Stripped";
            string copy=data;
            trim(data);
            AssertThat(data, Equals(copy));
        });
    });

});

int main(int argc, char** argv) {
    return bandit::run(argc, argv);
}
