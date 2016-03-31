/// Tests string functions

#include "string_functions.hpp"

#include <bandit/bandit.h>
#include <sstream>

go_bandit([]() {
  using namespace bandit;
  using namespace string_fun;
  using std::string;

  describe("trim", [&]() {
    it("1.1. Doesn't touch an empty string", [&] {
      string data = "Already Stripped";
      string copy = data;
      trim(data);
      AssertThat(data, Equals(copy));
    });
    it("1.2. Trims spaces and tabs the same", [&] {
      string data = "  \t  spaces and\t tabs\t";
      string copy = data;
      trim(data);
      AssertThat(data, Equals("spaces and\t tabs"));
    });
    it("1.3. Trim left", [&] {
      string data = "  \t  trim left";
      string copy = data;
      trim(data);
      AssertThat(data, Equals("trim left"));
    });
    it("1.4. Trim right", [&] {
      string data = "trim right  \t  ";
      string copy = data;
      trim(data);
      AssertThat(data, Equals("trim right"));
    });
  });

  describe("dequote_string", [&]() {
    std::string input = "", output = "";
    std::string::const_iterator end;

    before_each([&]() {
      input.clear();
      output.clear();
      end = output.end();
    });

    it("2.1. Handles empty input", [&] {
      input = "";
      end = dequoteString(input, output);
      AssertThat(output, IsEmpty());
    });
    it("2.2. No quotes", [&] {
      input = "This is a normal string";
      end = dequoteString(input, output);
      AssertThat(output, Is().EqualTo(input));
    });
    it("2.3. Converts a double backslash to a single", [&] {
      input = R"(This is a \\ normal string)";
      std::string expected = R"(This is a \ normal string)";
      end = dequoteString(input, output);
      AssertThat(output, Is().EqualTo(expected));
    });
    it("2.4. Converts an escaped x to a normal x", [&] {
      input = R"(This is a \x normal string)";
      std::string expected = R"(This is a x normal string)";
      end = dequoteString(input, output);
      AssertThat(output, Is().EqualTo(expected));
    });
    it("2.5. Escapes quotes correctly", [&] {
      input = R"(This is a \"normal\" string)";
      std::string expected = R"(This is a "normal" string)";
      end = dequoteString(input, output);
      AssertThat(output, Is().EqualTo(expected));
    });
  });
});

int main(int argc, char **argv) { return bandit::run(argc, argv); }
