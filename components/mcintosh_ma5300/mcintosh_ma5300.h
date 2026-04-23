#pragma once

#include "esphome/core/application.h"
#include "esphome/core/component.h"
#include "esphome/core/preferences.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/text_sensor/text_sensor.h"

#ifdef USE_MEDIA_PLAYER
#include "media_player/mcintosh_media_player.h"
#endif
#ifdef USE_SWITCH
#include "switch/mcintosh_switch.h"
#endif
#ifdef USE_NUMBER
#include "number/mcintosh_number.h"
#endif
#ifdef USE_SELECT
#include "select/mcintosh_select.h"
#endif
#ifdef USE_BUTTON
#include "button/mcintosh_button.h"
#endif
#ifdef USE_BINARY_SENSOR
#include "binary_sensor/mcintosh_binary_sensor.h"
#endif

#include <string>
#include <utility>
#include <vector>

namespace esphome {
namespace mcintosh_ma5300 {

class McIntoshMA5300 : public uart::UARTDevice, public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  float get_setup_priority() const override { return setup_priority::LATE; }

  // Text sensor setters
  void set_serial_number_sensor(text_sensor::TextSensor *s) { serial_number_sensor_ = s; }
  void set_firmware_version_sensor(text_sensor::TextSensor *s) { firmware_version_sensor_ = s; }
  void set_da_version_sensor(text_sensor::TextSensor *s) { da_version_sensor_ = s; }
  void set_last_error_sensor(text_sensor::TextSensor *s) { last_error_sensor_ = s; }

#ifdef USE_MEDIA_PLAYER
  void set_media_player(McIntoshMediaPlayer *mp) { media_player_ = mp; }
#endif
#ifdef USE_SWITCH
  void set_tone_switch(McIntoshSwitch *s)          { tone_switch_ = s; }
  void set_mono_switch(McIntoshSwitch *s)          { mono_switch_ = s; }
  void set_meter_lights_switch(McIntoshSwitch *s)  { meter_lights_switch_ = s; }
  void set_headphone_hxd_switch(McIntoshSwitch *s) { headphone_hxd_switch_ = s; }
#endif
#ifdef USE_NUMBER
  void set_bass_number(McIntoshNumber *n)       { bass_number_ = n; }
  void set_treble_number(McIntoshNumber *n)     { treble_number_ = n; }
  void set_balance_number(McIntoshNumber *n)    { balance_number_ = n; }
  void set_input_trim_number(McIntoshNumber *n) { input_trim_number_ = n; }
#endif
#ifdef USE_SELECT
  void set_input_select(McIntoshSelect *s)              { input_select_ = s; }
  void set_display_brightness_select(McIntoshSelect *s) { display_brightness_select_ = s; }
#endif
#ifdef USE_BINARY_SENSOR
  void set_headphones_sensor(McIntoshBinarySensor *s) { headphones_sensor_ = s; }
#endif
#ifdef USE_BUTTON
  void set_query_button(McIntoshButton *b) {
    b->set_action([this]() {
      ESP_LOGI("mcintosh_ma5300", "Manual QRY requested");
      this->send_command("QRY");
    });
  }
  void set_discover_inputs_button(McIntoshButton *b) {
    b->set_action([this]() { this->start_input_discovery_(); });
  }
  void set_volume_up_button(McIntoshButton *b) {
    b->set_action([this]() {
#ifdef USE_MEDIA_PLAYER
      if (media_player_ != nullptr) {
        int cur = (int) (media_player_->volume * McIntoshMediaPlayer::VOLUME_MAX + 0.5f);
        this->send_command("VOL " + to_string(std::min(McIntoshMediaPlayer::VOLUME_MAX, cur + 1)));
      }
#endif
    });
  }
  void set_volume_down_button(McIntoshButton *b) {
    b->set_action([this]() {
#ifdef USE_MEDIA_PLAYER
      if (media_player_ != nullptr) {
        int cur = (int) (media_player_->volume * McIntoshMediaPlayer::VOLUME_MAX + 0.5f);
        this->send_command("VOL " + to_string(std::max(0, cur - 1)));
      }
#endif
    });
  }
#endif

  void send_command(const std::string &cmd);
  void start_input_discovery_();

 protected:
  void parse_incoming_();
  void process_frame_(const std::string &frame);
  static const char *input_name_for_index_(int index);

  std::string rx_buffer_;

  // Current known input index (-1 = unknown)
  int current_input_{-1};

  // Input discovery state machine
  bool discovery_active_{false};
  int discovery_start_index_{-1};
  std::vector<std::pair<std::string, int>> discovered_inputs_;

  // Persisted input bitmask (bit N-1 = INP N enabled)
  ESPPreferenceObject input_pref_;
  uint16_t boot_input_mask_{0};

  text_sensor::TextSensor *serial_number_sensor_{nullptr};
  text_sensor::TextSensor *firmware_version_sensor_{nullptr};
  text_sensor::TextSensor *da_version_sensor_{nullptr};
  text_sensor::TextSensor *last_error_sensor_{nullptr};

#ifdef USE_MEDIA_PLAYER
  McIntoshMediaPlayer *media_player_{nullptr};
#endif
#ifdef USE_SWITCH
  McIntoshSwitch *tone_switch_{nullptr};
  McIntoshSwitch *mono_switch_{nullptr};
  McIntoshSwitch *meter_lights_switch_{nullptr};
  McIntoshSwitch *headphone_hxd_switch_{nullptr};
#endif
#ifdef USE_NUMBER
  McIntoshNumber *bass_number_{nullptr};
  McIntoshNumber *treble_number_{nullptr};
  McIntoshNumber *balance_number_{nullptr};
  McIntoshNumber *input_trim_number_{nullptr};
#endif
#ifdef USE_SELECT
  McIntoshSelect *input_select_{nullptr};
  McIntoshSelect *display_brightness_select_{nullptr};
#endif
#ifdef USE_BINARY_SENSOR
  McIntoshBinarySensor *headphones_sensor_{nullptr};
#endif
};

}  // namespace mcintosh_ma5300
}  // namespace esphome
