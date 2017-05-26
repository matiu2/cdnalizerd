#include <bandit/bandit.h>

#include <curlpp11.hpp>

#include <iostream>
#include <sstream>

#include <cstdlib>

#include "login.hpp"

using namespace bandit;
using namespace cdnalizerd;

std::pair<std::string, std::string> getCredentials() {
  // Get user's home directory
  char *username = std::getenv("OS_USERNAME");
  char *password = std::getenv("OS_PASSWORD");
  if ((username == nullptr) || (password == nullptr))
    throw std::runtime_error("Unable to login, please add the environment "
                             "varables. "
                             "eg.\nOS_USERNAME=abc\nOS_PASSWORD=5320203405430");
  return std::make_pair(username, password);
}

go_bandit([]() {

  curl::GlobalSentry curl;
   
  std::stringstream output;

  before_each([&]() { output.str(""); });

  describe("Login", [&]() {

    it("1.1. Can login", [&]() {
      auto creds = getCredentials();
      Rackspace rs;
      AssertThat(rs.token.empty(), snowhouse::Equals(true));
      rs.login(creds.first, creds.second);
      AssertThat(rs.token.empty(), snowhouse::Equals(false));
    });

  });

});

int main(int argc, char *argv[]) { return bandit::run(argc, argv); }
