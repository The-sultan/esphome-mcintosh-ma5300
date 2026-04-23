#include "mcintosh_switch.h"
#include "../mcintosh_ma5300.h"

namespace esphome {
namespace mcintosh_ma5300 {

void McIntoshSwitch::write_state(bool state) {
  hub_->send_command(state ? cmd_on_ : cmd_off_);
}

}  // namespace mcintosh_ma5300
}  // namespace esphome
