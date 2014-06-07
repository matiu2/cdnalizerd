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
        it("2. Trims spaces and tabs the same", [&] {
            string data="  \t  spaces and\t tabs\t";
            string copy=data;
            trim(data);
            AssertThat(data, Equals("spaces and\t tabs"));
        });
        it("3. Trim left", [&] {
            string data="  \t  trim left";
            string copy=data;
            trim(data);
            AssertThat(data, Equals("trim left"));
        });
        it("4. Trim right", [&] {
            string data="trim right  \t  ";
            string copy=data;
            trim(data);
            AssertThat(data, Equals("trim right"));
        });
    });

});

int main(int argc, char** argv) {
    return bandit::run(argc, argv);
}
