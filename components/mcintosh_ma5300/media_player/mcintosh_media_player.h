#pragma once

#include "esphome/core/component.h"
#include "esphome/components/media_player/media_player.h"

namespace esphome {
namespace mcintosh_ma5300 {

class McIntoshMA5300;

class McIntoshMediaPlayer : public media_player::MediaPlayer, public Component {
 public:
  // Maximum amp volume on the 0–100 scale exposed via the slider and step
  // buttons. Values above this are only reachable by direct RS232 commands.
  // Change to 100 to remove the limit entirely.
  static constexpr int VOLUME_MAX = 50;

  void set_hub(McIntoshMA5300 *hub) { hub_ = hub; }

  void setup() override {}
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::LATE; }

  media_player::MediaPlayerTraits get_traits() override;
  bool is_muted() const override { return muted_; }

  void publish_power(bool on);
  void publish_volume(int vol_pct);
  void publish_mute(bool muted);

 protected:
  void control(const media_player::MediaPlayerCall &call) override;

  McIntoshMA5300 *hub_{nullptr};
  bool muted_{false};
};

}  // namespace mcintosh_ma5300
}  // namespace esphome
