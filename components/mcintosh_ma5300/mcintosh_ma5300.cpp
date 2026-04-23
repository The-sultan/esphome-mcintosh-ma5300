#include "mcintosh_ma5300.h"
#include "esphome/core/log.h"

namespace esphome {
namespace mcintosh_ma5300 {

static const char *const TAG = "mcintosh_ma5300";

static const size_t RX_BUFFER_MAX = 512;

static bool starts_with(const std::string &s, const char *prefix) {
  size_t n = strlen(prefix);
  return s.size() >= n && s.compare(0, n, prefix) == 0;
}

// ─── Input name table ────────────────────────────────────────────────────────

const char *McIntoshMA5300::input_name_for_index_(int index) {
  static const char *const NAMES[] = {
    nullptr,      // 0 — unused
    "BAL",        // 1
    "UNBAL 1",    // 2
    "UNBAL 2",    // 3
    "UNBAL 3",    // 4
    "UNBAL 4",    // 5
    "MM PHONO",   // 6
    "COAX 1",     // 7
    "COAX 2",     // 8
    "OPTI 1",     // 9
    "OPTI 2",     // 10
    "USB",        // 11
    "MCT",        // 12
    "HDMI",       // 13
  };
  if (index >= 1 && index <= 13) return NAMES[index];
  return "UNKNOWN";
}

// ─── Component lifecycle ────────────────────────────────────────────────────

void McIntoshMA5300::setup() {
  ESP_LOGI(TAG, "Setting up McIntosh MA5300");

  // Restore previously discovered inputs (applied before HA connects).
  input_pref_ = global_preferences->make_preference<uint16_t>(fnv1_hash("mcintosh_ma5300_inputs"));
  input_pref_.load(&boot_input_mask_);
  if (boot_input_mask_ != 0) {
    uint16_t saved_mask = boot_input_mask_;
    std::vector<std::pair<std::string, int>> saved;
    for (int i = 1; i <= 13; i++) {
      if (saved_mask & (1u << (i - 1)))
        saved.emplace_back(input_name_for_index_(i), i);
    }
    ESP_LOGI(TAG, "Restoring %d saved input(s) from preferences", (int) saved.size());
#ifdef USE_SELECT
    if (input_select_ != nullptr)
      input_select_->set_discovered_options(saved);
#endif
  }

  this->set_timeout(2000, [this]() {
    ESP_LOGI(TAG, "Boot pref mask: 0x%04X (%s)", boot_input_mask_,
             boot_input_mask_ != 0 ? "loaded" : "empty/not found");
    ESP_LOGI(TAG, "Enabling automatic status notifications (STA 1)");
    this->send_command("STA 1");

    this->set_timeout(500, [this]() {
      ESP_LOGI(TAG, "Querying full state (QRY)");
      this->send_command("QRY");
    });
  });
}

void McIntoshMA5300::loop() { parse_incoming_(); }

void McIntoshMA5300::dump_config() {
  ESP_LOGCONFIG(TAG, "McIntosh MA5300:");
  this->check_uart_settings(115200, 1, uart::UART_CONFIG_PARITY_NONE, 8);
}

// ─── Command sending ─────────────────────────────────────────────────────────

void McIntoshMA5300::send_command(const std::string &cmd) {
  std::string wire = "(" + cmd + ")\r\n";
  ESP_LOGD(TAG, "TX: %s", cmd.c_str());
  this->write_str(wire.c_str());
}

// ─── Input discovery ─────────────────────────────────────────────────────────

void McIntoshMA5300::start_input_discovery_() {
#ifndef USE_SELECT
  return;
#else
  if (input_select_ == nullptr) return;
  if (current_input_ < 1) {
    ESP_LOGW(TAG, "Input discovery: current input unknown, skipping");
    return;
  }
  ESP_LOGI(TAG, "Starting input discovery (current: INP %d = %s)",
           current_input_, input_name_for_index_(current_input_));
  discovery_active_ = true;
  discovery_start_index_ = current_input_;
  discovered_inputs_.clear();
  discovered_inputs_.emplace_back(input_name_for_index_(current_input_), current_input_);
  this->send_command("INP U");
#endif
}

// ─── Parser ──────────────────────────────────────────────────────────────────

void McIntoshMA5300::parse_incoming_() {
  if (rx_buffer_.size() > RX_BUFFER_MAX) {
    ESP_LOGW(TAG, "RX buffer overflow (%d bytes), discarding", (int) rx_buffer_.size());
    rx_buffer_.clear();
  }

  while (this->available()) {
    uint8_t byte;
    this->read_byte(&byte);
    if (byte == 0x00) continue;
    rx_buffer_ += static_cast<char>(byte);
  }

  while (true) {
    auto open = rx_buffer_.find('(');
    if (open == std::string::npos) {
      rx_buffer_.clear();
      break;
    }
    if (open > 0) rx_buffer_.erase(0, open);

    auto close = rx_buffer_.find(')');
    if (close == std::string::npos) break;

    std::string frame = rx_buffer_.substr(1, close - 1);
    rx_buffer_.erase(0, close + 1);

    ESP_LOGV(TAG, "RX frame: %s", frame.c_str());
    process_frame_(frame);
  }
}

// ─── Frame dispatch ───────────────────────────────────────────────────────────

void McIntoshMA5300::process_frame_(const std::string &frame) {
  if (frame.empty()) return;

  // ── Boot identification ──────────────────────────────────────────────────
  if (frame == "MA5300") {
    ESP_LOGI(TAG, "Device identified: McIntosh MA5300");
    return;
  }

  // ── Info messages (boot / QRY response) ──────────────────────────────────
  if (starts_with(frame, "Serial Number: ")) {
    std::string value = frame.substr(15);
    ESP_LOGI(TAG, "Serial Number: %s", value.c_str());
    if (serial_number_sensor_ != nullptr) serial_number_sensor_->publish_state(value);
    return;
  }

  if (starts_with(frame, "FW Version: ")) {
    std::string value = frame.substr(12);
    ESP_LOGI(TAG, "FW Version: %s", value.c_str());
    if (firmware_version_sensor_ != nullptr) firmware_version_sensor_->publish_state(value);
    return;
  }

  if (starts_with(frame, "DA Version: ")) {
    std::string value = frame.substr(12);
    ESP_LOGI(TAG, "DA Version: %s", value.c_str());
    if (da_version_sensor_ != nullptr) da_version_sensor_->publish_state(value);
    return;
  }

  // ── Error responses ───────────────────────────────────────────────────────
  if (starts_with(frame, "ERROR - ")) {
    // During discovery an invalid-input error means we hit the end of the
    // available inputs before looping back — treat it as discovery complete.
    if (discovery_active_) {
      ESP_LOGW(TAG, "Error during input discovery, ending early: %s", frame.substr(8).c_str());
      discovery_active_ = false;
#ifdef USE_SELECT
      if (input_select_ != nullptr && !discovered_inputs_.empty())
        input_select_->set_discovered_options(discovered_inputs_);
#endif
      return;
    }
    std::string error = frame.substr(8);
    ESP_LOGW(TAG, "Device error: %s", error.c_str());
    if (last_error_sensor_ != nullptr) last_error_sensor_->publish_state(error);
    return;
  }

  // ── CMD [PAR] dispatch ────────────────────────────────────────────────────
  auto space = frame.find(' ');
  std::string cmd = (space == std::string::npos) ? frame : frame.substr(0, space);
  std::string param = (space == std::string::npos) ? "" : frame.substr(space + 1);

  // STA echo (amp confirms our own STA command)
  if (cmd == "STA") return;

  // Power
  if (cmd == "PWR") {
    bool on = (param == "1");
    ESP_LOGD(TAG, "Power: %s", on ? "ON" : "OFF");
#ifdef USE_MEDIA_PLAYER
    if (media_player_ != nullptr) media_player_->publish_power(on);
#endif
    return;
  }

  // Mute
  if (cmd == "MUT") {
    bool on = (param == "1");
    ESP_LOGD(TAG, "Mute: %s", on ? "ON" : "OFF");
#ifdef USE_MEDIA_PLAYER
    if (media_player_ != nullptr) media_player_->publish_mute(on);
#endif
    return;
  }

  // Input
  if (cmd == "INP") {
    auto idx = parse_number<int>(param);
    if (!idx.has_value()) {
      ESP_LOGW(TAG, "Invalid INP param: '%s'", param.c_str());
      return;
    }

    current_input_ = *idx;

    if (discovery_active_) {
      if (*idx == discovery_start_index_) {
        // Full loop complete.
        discovery_active_ = false;
        ESP_LOGI(TAG, "Input discovery complete — %d input(s) enabled:", (int) discovered_inputs_.size());
        for (const auto &inp : discovered_inputs_)
          ESP_LOGI(TAG, "  INP %d: %s", inp.second, inp.first.c_str());

        // Persist discovered inputs as a bitmask and reboot so HA sees the
        // filtered list on the next connection (HA caches options at connect time).
        uint16_t mask = 0;
        for (const auto &inp : discovered_inputs_)
          mask |= (1u << (inp.second - 1));
        input_pref_.save(&mask);
        global_preferences->sync();
        ESP_LOGI(TAG, "Input discovery saved (mask=0x%04X). Rebooting to apply...", mask);
        App.safe_reboot();
#ifdef USE_SELECT
        if (input_select_ != nullptr)
          input_select_->set_discovered_options(discovered_inputs_);
#endif
      } else {
        // New input found — record it and keep cycling.
        ESP_LOGD(TAG, "Discovery found INP %d: %s", *idx, input_name_for_index_(*idx));
        discovered_inputs_.emplace_back(input_name_for_index_(*idx), *idx);
        this->send_command("INP U");
      }
      return;
    }

    // Normal (non-discovery) input notification.
    ESP_LOGD(TAG, "Input: %d (%s)", *idx, input_name_for_index_(*idx));
#ifdef USE_SELECT
    if (input_select_ != nullptr && !input_select_->publish_by_index(*idx))
      ESP_LOGD(TAG, "INP %d not in select list, ignoring", *idx);
#endif
    return;
  }

  // Volume
  if (cmd == "VOL") {
    auto vol = parse_number<int>(param);
    if (!vol.has_value()) {
      ESP_LOGW(TAG, "Invalid VOL param: '%s'", param.c_str());
      return;
    }
    ESP_LOGD(TAG, "Volume: %d%%", *vol);
#ifdef USE_MEDIA_PLAYER
    if (media_player_ != nullptr) media_player_->publish_volume(*vol);
#endif
    return;
  }

  // Tone
  if (cmd == "TTN") {
    bool on = (param == "1");
    ESP_LOGD(TAG, "Tone: %s", on ? "ON" : "OFF");
#ifdef USE_SWITCH
    if (tone_switch_ != nullptr) tone_switch_->publish_state(on);
#endif
    return;
  }

  // Mono
  if (cmd == "TMO") {
    bool on = (param == "1");
    ESP_LOGD(TAG, "Mono: %s", on ? "ON" : "OFF");
#ifdef USE_SWITCH
    if (mono_switch_ != nullptr) mono_switch_->publish_state(on);
#endif
    return;
  }

  // Meter Lights
  if (cmd == "TML") {
    bool on = (param == "1");
    ESP_LOGD(TAG, "Meter Lights: %s", on ? "ON" : "OFF");
#ifdef USE_SWITCH
    if (meter_lights_switch_ != nullptr) meter_lights_switch_->publish_state(on);
#endif
    return;
  }

  // Headphone HXD
  if (cmd == "THH") {
    bool on = (param == "1");
    ESP_LOGD(TAG, "Headphone HXD: %s", on ? "ON" : "OFF");
#ifdef USE_SWITCH
    if (headphone_hxd_switch_ != nullptr) headphone_hxd_switch_->publish_state(on);
#endif
    return;
  }

  // Bass
  if (cmd == "TTB") {
    auto val = parse_number<int>(param);
    if (!val.has_value()) { ESP_LOGW(TAG, "Invalid TTB param: '%s'", param.c_str()); return; }
    ESP_LOGD(TAG, "Bass: %d dB", *val);
#ifdef USE_NUMBER
    if (bass_number_ != nullptr) bass_number_->publish_state((float) *val);
#endif
    return;
  }

  // Treble
  if (cmd == "TTT") {
    auto val = parse_number<int>(param);
    if (!val.has_value()) { ESP_LOGW(TAG, "Invalid TTT param: '%s'", param.c_str()); return; }
    ESP_LOGD(TAG, "Treble: %d dB", *val);
#ifdef USE_NUMBER
    if (treble_number_ != nullptr) treble_number_->publish_state((float) *val);
#endif
    return;
  }

  // Balance
  if (cmd == "TBA") {
    auto val = parse_number<int>(param);
    if (!val.has_value()) { ESP_LOGW(TAG, "Invalid TBA param: '%s'", param.c_str()); return; }
    ESP_LOGD(TAG, "Balance: %d", *val);
#ifdef USE_NUMBER
    if (balance_number_ != nullptr) balance_number_->publish_state((float) *val);
#endif
    return;
  }

  // Input Trim (protocol = half-dB steps; user sees dB)
  if (cmd == "TIN") {
    auto val = parse_number<int>(param);
    if (!val.has_value()) { ESP_LOGW(TAG, "Invalid TIN param: '%s'", param.c_str()); return; }
    float db = *val / 2.0f;
    ESP_LOGD(TAG, "Input Trim: %.1f dB", db);
#ifdef USE_NUMBER
    if (input_trim_number_ != nullptr) input_trim_number_->publish_state(db);
#endif
    return;
  }

  // Display Brightness
  if (cmd == "TDB") {
    auto val = parse_number<int>(param);
    if (!val.has_value()) { ESP_LOGW(TAG, "Invalid TDB param: '%s'", param.c_str()); return; }
    ESP_LOGD(TAG, "Display Brightness: %d", *val);
#ifdef USE_SELECT
    if (display_brightness_select_ != nullptr && !display_brightness_select_->publish_by_index(*val))
      ESP_LOGD(TAG, "TDB %d not in select list, ignoring", *val);
#endif
    return;
  }

  // Headphones
  if (cmd == "HPS") {
    bool on = (param == "1");
    ESP_LOGD(TAG, "Headphones: %s", on ? "connected" : "disconnected");
#ifdef USE_BINARY_SENSOR
    if (headphones_sensor_ != nullptr) headphones_sensor_->publish_state(on);
#endif
    return;
  }

  ESP_LOGD(TAG, "Unhandled frame: cmd='%s' param='%s'", cmd.c_str(), param.c_str());
}

}  // namespace mcintosh_ma5300
}  // namespace esphome
