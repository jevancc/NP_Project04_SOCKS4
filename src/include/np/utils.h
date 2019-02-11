#ifndef _NP_UTILS_H_
#define _NP_UTILS_H_

namespace np {
template <class T>
ssize_t SafeSend(int fd, const T& v) {
  const size_t kSendBufferSize = 512;
  for (size_t i = 0; i < v.size();) {
    size_t t = send(fd, &v[0] + i, min(kSendBufferSize, v.size() - i), 0);
    if (t != size_t(-1)) {
      i += t;
    } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
      // keep
    } else if (errno == EINTR) {
      // keep
    } else {
      return ssize_t(-1);
    }
  }
  return v.size();
}
}  // namespace np

#endif
