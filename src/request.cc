#include "np/socks/request.h"
#include <boost/array.hpp>
#include <boost/optional.hpp>
#include <limits>
using namespace std;

namespace np {
namespace socks {

bool Request::HostnameExists() const {
  return this->DSTIP > 0 && this->DSTIP <= numeric_limits<uint8_t>::max();
}

boost::optional<Request> Request::Parse(const uint8_t* buffer,
                                        ssize_t buffer_size) {
  ssize_t i;
  Request req;

  if (buffer_size < 9) {
    return boost::none;
  }

  // Parse VN
  if (buffer[0] != 0x4) {
    return boost::none;
  } else {
    req.VN = buffer[0];
  }

  // Parse CD
  if (buffer[1] != 1 && buffer[1] != 2) {
    return boost::none;
  } else {
    req.CD = buffer[1];
  }

  // Parse DSTPORT
  for (i = 2; i <= 3; i++) {
    req.DSTPORT = req.DSTPORT << 8 | buffer[i];
  }

  // Parse DSTIP
  for (i = 4; i <= 7; i++) {
    req.DSTIP = req.DSTIP << 8 | buffer[i];
  }

  // Parse USERID
  for (i = 8; i < buffer_size && buffer[i] != 0; i++) {
    req.USERID = req.USERID << 8 | buffer[i];
  }

  // SOCKS4a: Parse HOSTNAME
  if (req.HostnameExists() && i + 1 < buffer_size) {
    req.HOSTNAME.assign((char*)&buffer[i + 1], buffer_size - i - 1);
  }

  return req;
}

}  // namespace socks
}  // namespace np
