#include "mcintosh_select.h"
#include "../mcintosh_ma5300.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace mcintosh_ma5300 {

static const char *const TAG = "mcintosh_ma5300.select";

void McIntoshSelect::control(const std::string &value) {
  for (const auto &opt : options_) {
    if (opt.first == value) {
      std::string cmd = cmd_prefix_ + " " + to_string(opt.second);
      ESP_LOGD(TAG, "Requesting %s", cmd.c_str());
      hub_->send_command(cmd);
      return;
    }
  }
  ESP_LOGW(TAG, "Unknown option: '%s'", value.c_str());
}

bool McIntoshSelect::publish_by_index(int index) {
  for (const auto &opt : options_) {
    if (opt.second == index) {
      publish_state(opt.first);
      return true;
    }
  }
  return false;
}

void McIntoshSelect::set_discovered_options(const std::vector<std::pair<std::string, int>> &options) {
  options_ = options;
  // Build a FixedVector<const char*> from the persistent strings in options_.
  FixedVector<const char *> names;
  names.init(options_.size());
  for (const auto &opt : options_)
    names.push_back(opt.first.c_str());
  this->traits.set_options(names);
}

}  // namespace mcintosh_ma5300
}  // namespace esphome
