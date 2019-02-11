#include <array>
#include <boost/asio.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <regex>
#include <utility>
#include "np/firewall.h"
#include "np/socks/server.h"
using namespace std;

int main(int argc, char* const argv[]) {
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " [port]" << endl;
    return 1;
  }

  np::Firewall firewall;
  ifstream firewall_config("socks.conf");
  regex firewall_rule_regex(
      R"(^\s*permit\s*(b|c)\s*(\d+|\*).(\d+|\*).(\d+|\*).(\d+|\*)\s*$)");
  for (string s; getline(firewall_config, s);) {
    smatch sm;
    if (regex_match(s, sm, firewall_rule_regex)) {
      uint8_t type;
      uint32_t ip = 0;
      uint32_t mask = 0;

      type = sm[1] == "c" ? 1 : 2;
      for (int i = 2; i <= 5; i++) {
        if (sm[i] != "*") {
          ip |= stoi(sm[i]);
          mask |= 0xff;
        }

        if (i < 5) {
          ip <<= 8;
          mask <<= 8;
        }
      }
      firewall.AddRule(type, ip, mask);
    }
  }

  try {
    short port = atoi(argv[1]);
    boost::asio::io_service io_service;
    np::socks::Server server(io_service, port, firewall);
    cout << "Server is listening on port " << port << endl;
    server.Run();
  } catch (exception& e) {
    cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
