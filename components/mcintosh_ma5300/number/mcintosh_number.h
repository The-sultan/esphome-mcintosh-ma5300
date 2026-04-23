#pragma once

#include "esphome/components/number/number.h"

#include <string>

namespace esphome {
namespace mcintosh_ma5300 {

class McIntoshMA5300;

class McIntoshNumber : public number::Number {
 public:
  void set_hub(McIntoshMA5300 *hub) { hub_ = hub; }
  void set_command_prefix(const std::string &prefix) { cmd_prefix_ = prefix; }
  // For numbers where the protocol value differs from the user-facing value.
  // The amp receives round(user_value × multiplier).
  void set_multiplier(float m) { multiplier_ = m; }

 protected:
  void control(float value) override;

  McIntoshMA5300 *hub_{nullptr};
  std::string cmd_prefix_;
  float multiplier_{1.0f};
};

}  // namespace mcintosh_ma5300
}  // namespace esphome
