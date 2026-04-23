#include "mcintosh_number.h"
#include "../mcintosh_ma5300.h"

namespace esphome {
namespace mcintosh_ma5300 {

void McIntoshNumber::control(float value) {
  int protocol_val = (int) roundf(value * multiplier_);
  hub_->send_command(cmd_prefix_ + " " + to_string(protocol_val));
}

}  // namespace mcintosh_ma5300
}  // namespace esphome
