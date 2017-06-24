#include <iostream>
#include <sstream>
#include <stdexcept>

#include <cstdlib>

#include "Rackspace.hpp"

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

int testLogin(yield_context yield) {
  std::stringstream output;
  auto creds = getCredentials();
  int result = 0;
  Rackspace rs(yield);
  using namespace std;
  if (!rs.token().empty()) {
    ++result;
    cerr << "Expected RS token to be empty at the start" << endl;
  }
  rs.login(creds.first, creds.second);
  if (rs.token().empty()) {
    ++result;
    cerr << "Token is empty after login attempt" << endl;
  }
  cout << "Login successful. Token: " << rs.token() << endl;
  return result;
}

int main(int argc, char *argv[]) {
  int result;
  RESTClient::http::spawn(
      [&result, argc, argv](yield_context y) { result = testLogin(y); });
  RESTClient::http::run();
  return result;
}
