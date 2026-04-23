#include "mcintosh_media_player.h"
#include "../mcintosh_ma5300.h"
#include "esphome/core/log.h"

namespace esphome {
namespace mcintosh_ma5300 {

static const char *const TAG = "mcintosh_ma5300.media_player";

media_player::MediaPlayerTraits McIntoshMediaPlayer::get_traits() {
  auto traits = media_player::MediaPlayerTraits();
  traits.set_supports_pause(false);
  traits.set_supports_turn_off_on(true);
  // Remove streaming-only features that don't apply to an amplifier.
  traits.clear_feature_flags(
      media_player::MediaPlayerEntityFeature::BROWSE_MEDIA |
      media_player::MediaPlayerEntityFeature::PLAY_MEDIA |
      media_player::MediaPlayerEntityFeature::STOP |
      media_player::MediaPlayerEntityFeature::MEDIA_ANNOUNCE);
  // VOLUME_SET stays enabled for the slider (capped at 50% in control()).
  // Precise stepping above 50% is handled by dedicated button entities.
  return traits;
}

void McIntoshMediaPlayer::control(const media_player::MediaPlayerCall &call) {
  if (call.get_command().has_value()) {
    switch (*call.get_command()) {
      case media_player::MEDIA_PLAYER_COMMAND_TURN_ON:
        hub_->send_command("PWR 1");
        break;
      case media_player::MEDIA_PLAYER_COMMAND_TURN_OFF:
        hub_->send_command("PWR 0");
        break;
      case media_player::MEDIA_PLAYER_COMMAND_MUTE:
        hub_->send_command("MUT 1");
        break;
      case media_player::MEDIA_PLAYER_COMMAND_UNMUTE:
        hub_->send_command("MUT 0");
        break;
      case media_player::MEDIA_PLAYER_COMMAND_VOLUME_UP: {
        int next = std::min(VOLUME_MAX, (int) (this->volume * VOLUME_MAX + 0.5f) + 1);
        hub_->send_command("VOL " + to_string(next));
        break;
      }
      case media_player::MEDIA_PLAYER_COMMAND_VOLUME_DOWN: {
        int next = std::max(0, (int) (this->volume * VOLUME_MAX + 0.5f) - 1);
        hub_->send_command("VOL " + to_string(next));
        break;
      }
      default:
        break;
    }
  }
  if (call.get_volume().has_value()) {
    float received = *call.get_volume();
    int vol = (int) (received * VOLUME_MAX + 0.5f);
    ESP_LOGD(TAG, "Volume set: received=%.2f -> VOL %d", received, vol);
    hub_->send_command("VOL " + to_string(vol));
  }
}

void McIntoshMediaPlayer::publish_power(bool on) {
  this->state = on ? media_player::MEDIA_PLAYER_STATE_ON : media_player::MEDIA_PLAYER_STATE_OFF;
  this->publish_state();
}

void McIntoshMediaPlayer::publish_volume(int vol_pct) {
  this->volume = std::min(1.0f, (float) vol_pct / (float) VOLUME_MAX);
  this->publish_state();
}

void McIntoshMediaPlayer::publish_mute(bool muted) {
  muted_ = muted;
  this->publish_state();
}

void McIntoshMediaPlayer::dump_config() {
  ESP_LOGCONFIG(TAG, "McIntosh MA5300 Media Player");
}

}  // namespace mcintosh_ma5300
}  // namespace esphome
