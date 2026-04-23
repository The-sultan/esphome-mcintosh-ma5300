#pragma once

#include "esphome/components/switch/switch.h"

#include <string>

namespace esphome {
namespace mcintosh_ma5300 {

class McIntoshMA5300;

class McIntoshSwitch : public switch_::Switch {
 public:
  void set_hub(McIntoshMA5300 *hub) { hub_ = hub; }
  void set_command_on(const std::string &cmd) { cmd_on_ = cmd; }
  void set_command_off(const std::string &cmd) { cmd_off_ = cmd; }

 protected:
  void write_state(bool state) override;

  McIntoshMA5300 *hub_{nullptr};
  std::string cmd_on_;
  std::string cmd_off_;
};

}  // namespace mcintosh_ma5300
}  // namespace esphome
