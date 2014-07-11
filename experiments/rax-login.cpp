/// Experiment that logs in to Rackspace US API Servers

//#include "curl_easy.h"

#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <iterator>

#include <json.hpp>
#include <curl_easy.h>
#include <curl_writer.h>
#include <curl_sender.h>

int main(int argc, const char *argv[]) {
  using namespace json;

  // Get the username and api key from the command line
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " username apiKey" << std::endl;
    return -1;
  }
  std::string username(argv[1]);
  std::string apiKey(argv[2]);

  // The body is json
  JSON j(JList{JMap{
      {"auth", JMap{{"RAX-KSKEY:apiKeyCredentials",
                     JMap{{"username", username}, {"apiKey", apiKey}}}}}}});

  std::stringstream body;
  body << j;

  std::stringstream result;
  curl::curl_writer receiver(result);
  curl::curl_easy easy(receiver);
  curl::curl_sender<std::string> sender(easy);
  easy.add(curl_pair<CURLoption, std::string>(
      CURLOPT_URL, "https://identity.api.rackspacecloud.com/v2.0/tokens"));
  easy.add(curl_pair<CURLoption, int>(CURLOPT_POST, 1));
  easy.add(curl_pair<CURLoption, std::string>(CURLOPT_HTTPHEADER, "Content-type: application/json"));
  sender.send(body.str());
  easy.perform();

  std::cout << "Received: " << result << std::endl;
}
