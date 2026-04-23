#pragma once

#include "esphome/components/button/button.h"

#include <functional>

namespace esphome {
namespace mcintosh_ma5300 {

class McIntoshMA5300;

class McIntoshButton : public button::Button {
 public:
  void set_hub(McIntoshMA5300 *hub) { hub_ = hub; }
  void set_action(std::function<void()> action) { action_ = std::move(action); }

 protected:
  void press_action() override;

  McIntoshMA5300 *hub_{nullptr};
  std::function<void()> action_;
};

}  // namespace mcintosh_ma5300
}  // namespace esphome
