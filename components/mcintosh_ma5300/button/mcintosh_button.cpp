#include "mcintosh_button.h"
#include "esphome/core/log.h"

namespace esphome {
namespace mcintosh_ma5300 {

static const char *const TAG = "mcintosh_ma5300.button";

void McIntoshButton::press_action() {
  if (action_) {
    action_();
  } else {
    ESP_LOGW(TAG, "Button pressed but no action configured");
  }
}

}  // namespace mcintosh_ma5300
}  // namespace esphome
