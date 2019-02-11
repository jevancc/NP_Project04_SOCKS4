#include "np/socks/handler.h"
#include <boost/asio.hpp>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include "np/utils.h"
using namespace std;

namespace ip = boost::asio::ip;

namespace np {
namespace socks {

void ConnectHandler::Start() {
  auto self(shared_from_this());
  dst_socket_.async_connect(
      dst_ep_, [this, self](const boost::system::error_code& ec) {
        if (!ec) {
          self->Reply_(0x5A);
          self->PipeSocket_(src_socket_, dst_socket_, buffer_.data(),
                            buffer_.size() / 2);
          self->PipeSocket_(dst_socket_, src_socket_,
                            buffer_.data() + buffer_.size() / 2 + 1,
                            buffer_.size() - buffer_.size() / 2);
        } else {
          cerr << ec.message() << endl;
          self->Reply_(0x5B);
        }
      });
}

void ConnectHandler::Reply_(uint8_t code) {
  array<uint8_t, 8> data;
  auto dport = dst_ep_.port();
  auto daddr = dst_ep_.address().to_v4().to_ulong();
  auto p = data.data();

  p[0] = 0;
  p[1] = code;
  memcpy(&p[2], (uint8_t*)&dport, 2);
  memcpy(&p[4], (uint8_t*)&daddr, 4);
  SafeSend(src_socket_.native_handle(), data);
}

void ConnectHandler::PipeSocket_(ip::tcp::socket& src, ip::tcp::socket& dst,
                                 uint8_t* buffer, ssize_t buffer_size) {
  auto self(shared_from_this());
  src.async_read_some(
      boost::asio::buffer(buffer, buffer_size),
      [=, &src, &dst](boost::system::error_code ec, size_t bytes_transferred) {
        if (!ec) {
          dst.async_send(boost::asio::buffer(buffer, bytes_transferred),
                         [=, &src, &dst](boost::system::error_code ec,
                                         size_t bytes_transferred) {
                           if (!ec) {
                             self->PipeSocket_(src, dst, buffer, buffer_size);
                           }
                         });
        }
      });
}

void BindHandler::Start() {
  if (bind_acceptor_.is_open()) {
    auto ep = bind_acceptor_.local_endpoint();
    ep.address(ip::address_v4((uint_least32_t)0));
    Reply_(0x5A, ep);
    DoAccept_();
  } else {
    Reply_(0x5B, dst_ep_);
  }
}

void BindHandler::DoAccept_() {
  auto self(shared_from_this());
  bind_acceptor_.async_accept(
      bind_socket_, [this, self](boost::system::error_code ec) {
        if (bind_acceptor_.is_open() && !ec) {
          auto ep = bind_socket_.remote_endpoint();
          if (ep.address() == dst_ep_.address()) {
            self->Reply_(0x5A, ep);
            self->PipeSocket_(src_socket_, bind_socket_, buffer_.data(),
                              buffer_.size() / 2);
            self->PipeSocket_(bind_socket_, src_socket_,
                              buffer_.data() + buffer_.size() / 2 + 1,
                              buffer_.size() - buffer_.size() / 2);
          } else {
            self->Reply_(0x5B, ep);
          }
        } else {
          if (ec) {
            cerr << ec.message() << endl;
          }
          self->Reply_(0x5B, dst_ep_);
        }
      });
}

void BindHandler::Reply_(uint8_t code, const ip::tcp::endpoint& ep) {
  array<uint8_t, 8> data;
  auto dport = ep.port();
  auto daddr = ep.address().to_v4().to_ulong();
  auto p = data.data();

  p[0] = 0;
  p[1] = code;
  memcpy(&p[2], (uint8_t*)&dport, 2);
  memcpy(&p[4], (uint8_t*)&daddr, 4);
  SafeSend(src_socket_.native_handle(), data);
}

void BindHandler::PipeSocket_(ip::tcp::socket& src, ip::tcp::socket& dst,
                              uint8_t* buffer, ssize_t buffer_size) {
  auto self(shared_from_this());
  src.async_read_some(
      boost::asio::buffer(buffer, buffer_size),
      [=, &src, &dst](boost::system::error_code ec, size_t bytes_transferred) {
        if (!ec) {
          dst.async_send(boost::asio::buffer(buffer, bytes_transferred),
                         [=, &src, &dst](boost::system::error_code ec,
                                         size_t bytes_transferred) {
                           if (!ec) {
                             self->PipeSocket_(src, dst, buffer, buffer_size);
                           }
                         });
        }
      });
}

}  // namespace socks
}  // namespace np
