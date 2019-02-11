#ifndef _NP_SOCKS_CONNECTION_H_
#define _NP_SOCKS_CONNECTION_H_
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
#include <cstdint>
#include <iostream>
#include <memory>
#include "np/firewall.h"
#include "np/socks/request.h"
using namespace std;
namespace ip = boost::asio::ip;

namespace np {
namespace socks {

class Connection : public enable_shared_from_this<Connection> {
 private:
  boost::asio::io_service& io_service_;
  boost::asio::ip::tcp::socket socket_;
  ip::tcp::resolver resolver_;
  boost::array<uint8_t, 4096> buffer_;
  Request request_;
  const Firewall& firewall_;

  void DoRead_();
  void ResolveHost_();
  void Handle_(boost::asio::ip::tcp::endpoint ep);
  void Reply_(uint8_t code, const ip::tcp::endpoint& ep);

 public:
  Connection(boost::asio::io_service& io_service,
             boost::asio::ip::tcp::socket socket, const Firewall& firewall)
      : io_service_(io_service),
        socket_(move(socket)),
        resolver_(io_service),
        firewall_(firewall){};

  void Stop() { socket_.close(); }
  void Start() { DoRead_(); };
};
}  // namespace socks
}  // namespace np

#endif
