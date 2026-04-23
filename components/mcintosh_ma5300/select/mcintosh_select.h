#pragma once

#include "esphome/components/select/select.h"

#include <string>
#include <utility>
#include <vector>

namespace esphome {
namespace mcintosh_ma5300 {

class McIntoshMA5300;

class McIntoshSelect : public select::Select {
 public:
  void set_hub(McIntoshMA5300 *hub) { hub_ = hub; }
  void set_command_prefix(const std::string &prefix) { cmd_prefix_ = prefix; }
  void add_option(const std::string &name, int index) { options_.emplace_back(name, index); }

  // Called by the hub after input discovery completes.
  void set_discovered_options(const std::vector<std::pair<std::string, int>> &options);

  // Called by the hub when it receives (INP n). Returns false if n is not in the option list.
  bool publish_by_index(int index);

 protected:
  void control(const std::string &value) override;

  McIntoshMA5300 *hub_{nullptr};
  std::string cmd_prefix_{"INP"};
  std::vector<std::pair<std::string, int>> options_;
};

}  // namespace mcintosh_ma5300
}  // namespace esphome
