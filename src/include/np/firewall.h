#ifndef _NP_FIREWALL_H_
#define _NP_FIREWALL_H_

#include <tuple>
#include <vector>
using namespace std;

namespace np {
class Firewall {
 private:
  vector<tuple<uint8_t, uint32_t, uint32_t>> rules_;

 public:
  void AddRule(uint8_t type, uint32_t ip, uint32_t mask) {
    rules_.push_back(make_tuple(type, ip, mask));
  }

  bool Check(uint8_t type, uint32_t ip) const {
    for (const auto& rule : rules_) {
      uint8_t type_;
      uint32_t ip_, mask_;
      tie(type_, ip_, mask_) = rule;
      if (type == type_ && (ip ^ ip_) & mask_) {
        return false;
      }
    }
    return true;
  }
};
}  // namespace np

#endif
