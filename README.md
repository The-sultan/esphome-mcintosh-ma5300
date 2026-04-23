# ESPHome — McIntosh MA5300

An ESPHome external component for controlling the **McIntosh MA5300** integrated amplifier via RS-232.

Exposes the amplifier as a full Home Assistant integration with a media player entity plus individual controls for every parameter the MA5300 supports over its serial protocol.

---

## Hardware

| Item | Detail |
|---|---|
| Microcontroller | ESP32 (tested on ESP32 DevKit) |
| Serial adapter | RS-232 ↔ TTL (MAX3232 or equivalent) |
| TX pin | GPIO 25 |
| RX pin | GPIO 26 |
| Baud rate | 115 200 8N1 |

Connect the RS-232 adapter between the ESP32 UART pins and the **RS-232** port on the rear of the MA5300.

---

## Features

| Entity | Type | Default |
|---|---|---|
| MA5300 | Media Player (power, volume, mute) | Enabled |
| Input | Select | Enabled |
| Volume Up / Volume Down | Button | Enabled |
| Tone / Mono / Meter Lights / Headphone HXD | Switch | Disabled |
| Bass / Treble / Balance / Input Trim | Number | Disabled |
| Display Brightness | Select | Disabled |
| Headphones | Binary Sensor | Disabled |
| Serial Number / Firmware Version / DA Version | Text Sensor | Disabled |
| Last Error | Text Sensor | Disabled |
| Query State / Discover Inputs | Button | Disabled |

Disabled entities are hidden by default in Home Assistant and can be enabled individually as needed.

---

## Installation

Add to your ESPHome YAML:

```yaml
external_components:
  - source: github://The-sultan/esphome-mcintosh-ma5300
    components: [mcintosh_ma5300]
```

---

## Configuration

```yaml
uart:
  id: mc_uart
  tx_pin: GPIO25
  rx_pin: GPIO26
  baud_rate: 115200
  data_bits: 8
  parity: NONE
  stop_bits: 1

mcintosh_ma5300:
  id: mc
  uart_id: mc_uart

media_player:
  - platform: mcintosh_ma5300
    mcintosh_ma5300_id: mc
    name: "MA5300"

select:
  - platform: mcintosh_ma5300
    mcintosh_ma5300_id: mc
    type: input
    name: "Input"

button:
  - platform: mcintosh_ma5300
    mcintosh_ma5300_id: mc
    type: volume_up
    name: "Volume Up"
  - platform: mcintosh_ma5300
    mcintosh_ma5300_id: mc
    type: volume_down
    name: "Volume Down"
```

See [`example.yaml`](example.yaml) for the full configuration with all entities.

---

## Input Discovery

The MA5300 only activates the inputs that are physically connected. The **Discover Inputs** button cycles through all inputs and builds a filtered list, which is persisted to flash. After discovery the device reboots once so Home Assistant receives the filtered input list on reconnect.

To reset the input list back to all 13 inputs, erase the device flash:

```
esptool --chip esp32 --port /dev/tty.XXXX erase-flash
```

---

## Volume

The media player slider is limited to **50% of the amp's range** (VOL 0–50) by the `VOLUME_MAX` constant in [mcintosh_media_player.h](components/mcintosh_ma5300/media_player/mcintosh_media_player.h). Change that value if a higher ceiling is needed. The Volume Up / Volume Down buttons step by 1 unit regardless of slider position.

---

## Supported MA5300 Protocol Commands

| Command | Description |
|---|---|
| `PWR` | Power on/off |
| `VOL` | Volume (0–100) |
| `MUT` | Mute |
| `INP` | Input selection |
| `TTN` | Tone control on/off |
| `TMO` | Mono on/off |
| `TML` | Meter lights on/off |
| `THH` | Headphone HXD on/off |
| `TTB` | Bass (−12 to +12 dB) |
| `TTT` | Treble (−12 to +12 dB) |
| `TBA` | Balance (−50 to +50) |
| `TIN` | Input trim (−6.0 to +6.0 dB, 0.5 dB steps) |
| `TDB` | Display brightness (0–4) |
| `HPS` | Headphones connected/disconnected |
| `QRY` | Query full state |
| `STA` | Enable automatic status notifications |
