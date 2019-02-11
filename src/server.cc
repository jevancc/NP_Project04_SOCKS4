#include "np/socks/server.h"
#include <memory>
#include "np/socks/connection.h"
using namespace std;
using namespace boost::asio::ip;

namespace np {
namespace socks {

Server::Server(boost::asio::io_service& io_service, unsigned short port,
               const Firewall& firewall)
    : io_service_(io_service),
      signal_(io_service, SIGCHLD),
      acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
      socket_(io_service),
      firewall_(firewall) {}

void Server::Run() {
  DoSignalWait_();
  DoAccept_();
  io_service_.run();
}

void Server::DoSignalWait_() {
  this->signal_.async_wait(
      [this](const boost::system::error_code& ec, int signum) {
        if (this->acceptor_.is_open()) {
          int status = 0;
          while (waitpid(-1, &status, WNOHANG) > 0)
            ;

          this->DoSignalWait_();
        }
      });
}

void Server::DoAccept_() {
  acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
    if (!acceptor_.is_open()) {
      return;
    } else if (!ec) {
      make_shared<Connection>(io_service_, move(socket_), firewall_)->Start();
    } else {
      cerr << "Accept error: " << ec.message() << std::endl;
    }
    this->DoAccept_();
  });
}

}  // namespace socks
}  // namespace np
