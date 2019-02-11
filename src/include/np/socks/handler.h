#ifndef _NP_SOCKS_HANDLER_H_
#define _NP_SOCKS_HANDLER_H_

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <cstdint>
#include <memory>
#include "np/socks/connection.h"
using namespace std;

namespace np {
namespace socks {

class Connection;
class ConnectHandler : public enable_shared_from_this<ConnectHandler> {
 private:
  boost::asio::ip::tcp::socket src_socket_;
  boost::asio::ip::tcp::socket dst_socket_;
  boost::asio::ip::tcp::endpoint dst_ep_;
  boost::array<uint8_t, 8192> buffer_;

  void PipeSocket_(boost::asio::ip::tcp::socket& src,
                   boost::asio::ip::tcp::socket& dst, uint8_t* buffer,
                   ssize_t buffer_size);
  void Reply_(uint8_t code);

 public:
  ConnectHandler(boost::asio::io_service& io_service,
                 boost::asio::ip::tcp::socket src_socket,
                 boost::asio::ip::tcp::endpoint dst_ep)
      : src_socket_(move(src_socket)),
        dst_socket_(io_service),
        dst_ep_(dst_ep) {}

  void Start();
};

class BindHandler : public enable_shared_from_this<BindHandler> {
 private:
  boost::asio::ip::tcp::socket src_socket_;
  boost::asio::ip::tcp::socket bind_socket_;
  boost::asio::ip::tcp::acceptor bind_acceptor_;
  boost::asio::ip::tcp::endpoint dst_ep_;
  boost::array<uint8_t, 8192> buffer_;

  void DoAccept_();
  void PipeSocket_(boost::asio::ip::tcp::socket& src,
                   boost::asio::ip::tcp::socket& dst, uint8_t* buffer,
                   ssize_t buffer_size);
  void Reply_(uint8_t code, const boost::asio::ip::tcp::endpoint& ep);

 public:
  BindHandler(boost::asio::io_service& io_service,
              boost::asio::ip::tcp::socket src_socket,
              boost::asio::ip::tcp::endpoint dst_ep)
      : src_socket_(move(src_socket)),
        bind_socket_(io_service),
        bind_acceptor_(io_service, boost::asio::ip::tcp::endpoint(
                                       boost::asio::ip::tcp::v4(), 0)),
        dst_ep_(dst_ep) {}
  void Start();
};

}  // namespace socks
}  // namespace np

#endif
