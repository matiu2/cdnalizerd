#include "login.hpp"

#include <boost/asio/deadline_timer.hpp>
#include <boost/system/system_error.hpp>

#include <RESTClient/tcpip/interface.hpp>

namespace cdnalizerd {

// Fills the cache of all accounts - spawns more cooperative threads and waits for them
void login(yield_context &yield, AccountCache& accounts, const Config& config) {
  // Spawn some workers to fill in the account info (token and urls)
  std::clog << "INFO: Getting API Authentication tokens..." << std::endl;
  boost::asio::deadline_timer waitForLogins(*RESTClient::tcpip::getService(),
                                            boost::posix_time::minutes(10));
  int loginWorkers = 2;
  for (int i = 0; i != loginWorkers; ++i) {
    RESTClient::http::spawn([&](yield_context y) {
      fillAccountCache(y, config, accounts, [&loginWorkers, &waitForLogins]() {
        --loginWorkers;
        if (loginWorkers == 0) {
          boost::system::error_code ec;
          waitForLogins.cancel(ec);
        }
      });
    });
  }

  // Wait for all the logins to finish
  try {
    waitForLogins.async_wait(yield);
  } catch(boost::system::system_error& e) {
    // This should throw operation_aborted, because we cancel the timer once all the logins are complete
    if (e.code() != boost::asio::error::operation_aborted)
      throw e;
  }

  // Logins to Rackspace timed out if this is not 0
  assert(loginWorkers == 0);

}

  
} /* cdnalizerd  */ 
