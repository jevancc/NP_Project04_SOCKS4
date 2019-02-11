#ifndef _NP_SOCKS_REQUEST_H_
#define _NP_SOCKS_REQUEST_H_

#include <boost/array.hpp>
#include <boost/optional.hpp>
#include <cstdint>
#include <string>
using namespace std;

namespace np {
namespace socks {

class Request {
 public:
  enum { kUnknown = 0, kConnect = 1, kBind = 2 };
  uint8_t VN;
  uint8_t CD;
  uint16_t DSTPORT;
  uint32_t DSTIP;
  uint64_t USERID;
  string HOSTNAME;

  Request()
      : VN(0), CD(kUnknown), DSTPORT(0), DSTIP(0), USERID(0), HOSTNAME() {}
  static boost::optional<Request> Parse(const uint8_t* buffer,
                                        ssize_t buffer_size);
  bool HostnameExists() const;
};

}  // namespace socks
}  // namespace np

#endif
