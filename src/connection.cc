#define BOOST_ASIO_DECL
#include "np/socks/connection.h"
#include <algorithm>
#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <memory>
#include <string>
#include <vector>
#include "fmt/core.h"
#include "np/firewall.h"
#include "np/socks/handler.h"
#include "np/socks/request.h"
#include "np/utils.h"
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
#undef BOOST_NO_CXX11_SCOPED_ENUMS
using namespace std;
namespace ip = boost::asio::ip;

namespace np {
namespace socks {

void Connection::DoRead_() {
  auto self(shared_from_this());

  fill(buffer_.begin(), buffer_.end(), 0);
  socket_.async_read_some(
      boost::asio::buffer(buffer_, buffer_.size()),
      [this, self](boost::system::error_code ec, size_t bytes_transferred) {
        auto req = Request::Parse(buffer_.data(), bytes_transferred);

        if (req == boost::none) {
          return;
        } else {
          request_ = move(*req);
        }

        fmt::print(
            "<S_IP>: {}\n"
            "<S_PORT>: {}\n"
            "<D_IP>: {}\n"
            "<D_PORT>: {}\n"
            "<Command>: {}\n"
            "<Reply>: Accept\n",
            socket_.remote_endpoint().address().to_string(),
            socket_.remote_endpoint().port(),
            ip::address_v4((uint_least32_t)request_.DSTIP).to_string(),
            request_.DSTPORT,
            request_.CD == Request::kConnect ? "Connect" : "Bind");

        ResolveHost_();
      });
}

void Connection::ResolveHost_() {
  auto self(shared_from_this());

  if (request_.HostnameExists()) {
    ip::tcp::resolver::query query(request_.HOSTNAME);
    resolver_.async_resolve(
        query, [this, self](const boost::system::error_code& ec,
                            ip::tcp::resolver::iterator it) {
          if (!ec) {
            auto ep = it->endpoint();
            ep.port(request_.DSTPORT);
            self->Handle_(move(ep));
          } else {
            cerr << ec.message() << endl;
            self->Reply_(
                0x5B, ip::basic_endpoint<ip::tcp>(
                          ip::address_v4((uint_least32_t)self->request_.DSTIP),
                          self->request_.DSTPORT));
          }
        });
  } else {
    Handle_(ip::basic_endpoint<ip::tcp>(
        ip::address_v4((uint_least32_t)request_.DSTIP),
        (unsigned short)request_.DSTPORT));
  }
}

void Connection::Handle_(boost::asio::ip::tcp::endpoint ep) {
  if (firewall_.Check(request_.CD, ep.address().to_v4().to_ulong())) {
    if (request_.CD == Request::kConnect) {
      make_shared<ConnectHandler>(io_service_, move(this->socket_), move(ep))
          ->Start();
    } else if (request_.CD == Request::kBind) {
      make_shared<BindHandler>(io_service_, move(this->socket_), move(ep))
          ->Start();
    }
  } else {
    Reply_(0x5B, ep);
  }
}

void Connection::Reply_(uint8_t code, const ip::tcp::endpoint& ep) {
  array<uint8_t, 8> data;
  auto dport = ep.port();
  auto daddr = ep.address().to_v4().to_ulong();
  auto p = data.data();

  p[0] = 0;
  p[1] = code;
  memcpy(&p[2], (uint8_t*)&dport, 2);
  memcpy(&p[4], (uint8_t*)&daddr, 4);
  SafeSend(socket_.native_handle(), data);
}

}  // namespace socks
}  // namespace np
