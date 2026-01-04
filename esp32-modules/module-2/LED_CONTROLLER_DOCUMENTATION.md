# LED Controller Module Documentation

## Overview

The LED Controller module is an ESP32-based multi-strip LED controller that manages up to 5 independent LED strips (all WS2815/WS2812 RGBW strips). It supports button control, motion sensor activation, dimming, transitions, synchronized control for kitchen lighting, and MQTT/WebSocket integration for remote control.

## Hardware Specifications

- **Microcontroller**: ESP32
- **LED Types**:
  - WS2815 RGBW (12V) - Strips 0-3
  - WS2812 RGBW (12V, GRBW protocol) - Strip 4 (Bedroom)
- **Number of Strips**: 5
- **RMT Channels**: Each strip uses a separate RMT channel (RMT0-RMT4)

## LED Strips Configuration

| Strip Index | Pin | LED Count | RMT Channel | LED Type | Description                    | Control                                |
| ----------- | --- | --------- | ----------- | -------- | ------------------------------ | -------------------------------------- |
| **Strip 0** | 33  | 44        | RMT0        | RGBW     | Kitchen (main)                 | Button 1 (Pin 4)                       |
| **Strip 1** | 18  | 178       | RMT1        | RGBW     | Main lighting                  | Button 2 (Pin 12)                      |
| **Strip 2** | 19  | 23        | RMT2        | RGBW     | Kitchen extension (spice rack) | Auto-synced with Strip 0               |
| **Strip 3** | 25  | 53        | RMT3        | RGBW     | Bathroom                       | Button 4 (Pin 13) + PIR sensor (Pin 2) |
| **Strip 4** | 5   | 30        | RMT4        | RGBW     | Bedroom (GRBW protocol)        | Button 4 (Pin 13)                      |

### Strip Synchronization

- **Strip 0 and Strip 2** are synchronized in software
- When Strip 0 is turned on/off, Strip 2 automatically follows
- Both strips use the same brightness, transitions, and dimming
- They use different RMT channels but are controlled together

### Strip 3 (Bathroom) - Motion-Activated with Manual Control

Strip 3 has three operating modes:

- **OFF**: Strip is off, motion sensor is ignored
- **AUTO**: Motion sensor controls the strip (default behavior)
  - Automatically turns on when motion is detected
  - Resets timeout timer on each motion detection
  - Automatically turns off after 60 seconds of no motion
  - Remembers last brightness setting (default: 50%)
- **ON**: Strip stays on, motion sensor is ignored

The mode can be changed via:

- Frontend UI (3-position button: OFF/AUTO/ON)
- MQTT command: `smartcamper/commands/led-controller/strip/3/mode` with payload `{"mode": "OFF"|"AUTO"|"ON"}`

## Buttons Configuration

| Button       | Pin | Controls          | Description                                          |
| ------------ | --- | ----------------- | ---------------------------------------------------- |
| **Button 1** | 4   | Strip 0 + Strip 2 | Kitchen - toggles main and extension strips together |
| **Button 2** | 12  | Strip 1           | Main lighting control                                |
| **Button 3** | 27  | Floor lighting    | Floor lighting relay control (toggle only)           |
| **Button 4** | 13  | Strip 4           | Bedroom lighting control                             |

### Button Functions

- **Click**: Toggle strip ON/OFF (with random transitions)
- **Hold** (>250ms): Start dimming (increase/decrease brightness) - _Not available for Button 3_
- **Button 3**: Simple toggle for floor lighting relay (no dimming)
- **Button 4**: Full control for Strip 4 (Bedroom) - same as Button 2

## Additional Circuits

### Floor Lighting (Relay Circuit)

| Component    | Pin | Direction | Description                  |
| ------------ | --- | --------- | ---------------------------- |
| **Relay**    | 26  | Output    | Floor lighting relay control |
| **Button 3** | 27  | Input     | Floor lighting toggle button |

**Floor Lighting Features:**

- Controlled via relay (GPIO 26)
- Toggle button on GPIO 27
- Simple ON/OFF control (no dimming)
- Uses standard LED diodes (not LED strips)
- Independent from LED strip controls

## Sensors Configuration

| Sensor                           | Pin | Controls           | Settings                       |
| -------------------------------- | --- | ------------------ | ------------------------------ |
| **PIR Motion Sensor (HC-SR501)** | 2   | Strip 3 (Bathroom) | Timeout: 60 seconds (1 minute) |

### Motion Sensor Behavior

- Only active when Strip 3 is in **AUTO** mode
- Automatically turns on Strip 3 when motion is detected
- Resets timeout timer on each motion detection
- Automatically turns off after 60 seconds of no motion
- Motion sensor is completely ignored in OFF and ON modes

## Color Configuration

- **All Strips (RGBW)**: White using RGB+W channels
  - All channels (R, G, B, W) are set to the same brightness value
  - Provides full-spectrum white light
- **Strip 4 (Bedroom)**: Uses warm white color
  - Special warm white configuration for cozy bedroom lighting
  - Warm white proportions: R=100%, G=90%, B=75%
  - Creates warmer, more comfortable light for bedroom environment

## Brightness Settings

- **Minimum Brightness**: 1
- **Maximum Brightness**: 255
- **Default Brightness**: 128 (50% on first power on)
- **Dimming Speed**: 50 brightness units per second

## Features

### Transitions

The controller supports 8 different transition effects:

**Turn On Transitions:**

1. Center to Edges - Smoothly expands from center
2. Random LEDs - Random LEDs turn on sequentially
3. Left to Right - Sequential activation from left
4. Edges to Center - Expands from edges to center

**Turn Off Transitions:**

1. Edges to Center - Turns off from edges
2. Random LEDs - Random LEDs turn off sequentially
3. Left to Right - Sequential deactivation from left
4. Center to Edges - Turns off from center outward

- Transition duration: 1 second
- Randomly selected for each on/off action

### Dimming

- Activated by holding button for >250ms
- Smooth brightness increase/decrease
- Alternates between increasing and decreasing
- Blinks when reaching maximum brightness
- Available for all strips except Strip 3 in AUTO mode (dimming works in OFF/ON modes)

### Blinking

- Occurs when brightness reaches maximum
- Duration: 300ms
- Minimum brightness during blink: 30% of current brightness
- Visual feedback for maximum brightness

## Communication Protocol

### MQTT Topics

**Status Publishing:**

- `smartcamper/sensors/led-controller/status` - Unified JSON with all strips and relays status

**Command Topics:**

- `smartcamper/commands/led-controller/strip/{index}/on` - Turn on strip
- `smartcamper/commands/led-controller/strip/{index}/off` - Turn off strip
- `smartcamper/commands/led-controller/strip/{index}/toggle` - Toggle strip
- `smartcamper/commands/led-controller/strip/{index}/brightness` - Set brightness (payload: `{"value": 0-255}`)
- `smartcamper/commands/led-controller/strip/3/mode` - Set Strip 3 mode (payload: `{"mode": "OFF"|"AUTO"|"ON"}`)
- `smartcamper/commands/led-controller/relay/toggle` - Toggle relay

### MQTT Message Format

**Status Message (JSON):**

```json
{
  "strips": {
    "0": { "state": "ON", "brightness": 128 },
    "1": { "state": "OFF", "brightness": 128 },
    "2": { "state": "ON", "brightness": 128 },
    "3": { "state": "ON", "brightness": 128, "mode": "AUTO" },
    "4": { "state": "OFF", "brightness": 128 }
  },
  "relays": {
    "0": { "state": "ON" }
  }
}
```

**Technical Details:**

- MQTT buffer size: 1024 bytes
- JSON document size: 1024 bytes
- Status published every 10 seconds (heartbeat interval)
- Status also published on any state change

## Pin Summary

| Component      | Pin | Direction       | Description                 |
| -------------- | --- | --------------- | --------------------------- |
| **Strip 0**    | 33  | Output          | Kitchen main LED strip      |
| **Strip 1**    | 18  | Output          | Main lighting LED strip     |
| **Strip 2**    | 19  | Output          | Kitchen extension LED strip |
| **Strip 3**    | 25  | Output          | Bathroom LED strip          |
| **Strip 4**    | 5   | Output          | Bedroom LED strip           |
| **Button 1**   | 4   | Input (Pull-up) | Kitchen control button      |
| **Button 2**   | 12  | Input (Pull-up) | Main lighting button        |
| **Button 3**   | 27  | Input (Pull-up) | Floor lighting button       |
| **Button 4**   | 13  | Input (Pull-up) | Bedroom control button      |
| **Relay**      | 26  | Output          | Floor lighting relay        |
| **PIR Sensor** | 2   | Input           | Motion detection sensor     |

## Technical Details

### RMT Channels

Each LED strip requires a dedicated RMT (Remote Control) channel on ESP32:

- Strip 0: RMT0 (WS2815 RGBW)
- Strip 1: RMT1 (WS2815 RGBW)
- Strip 2: RMT2 (WS2815 RGBW)
- Strip 3: RMT3 (WS2815 RGBW)
- Strip 4: RMT4 (WS2812 RGBW, GRBW protocol)

This allows independent timing control for each strip.

### Debouncing

- Button debounce delay: 50ms
- Separate debounce state machine for each button

### State Management

Each strip maintains:

- On/Off state
- Current brightness level
- Dimming state and direction
- Transition state
- Blink state
- Mode (for Strip 3: OFF/AUTO/ON)
- Last auto brightness (for Strip 3 in AUTO mode)

### Network Features

- **Non-blocking WiFi**: WiFi connection attempts do not block main loop
- **Local controls work offline**: Buttons, PIR sensor, and LED control work even without WiFi/MQTT
- **Auto-reconnect**: Automatic reconnection to WiFi and MQTT when connection is lost
- **MQTT QoS**: Level 0 (at most once delivery)

## Usage Examples

### Turning On Kitchen Lights

1. Press Button 1 (Pin 4)
2. Strip 0 turns on with random transition
3. Strip 2 automatically turns on simultaneously
4. Both strips maintain synchronized brightness

### Dimming Main Lighting

1. Press and hold Button 2 (Pin 12)
2. Brightness starts changing (increase/decrease alternates)
3. Release button to stop dimming
4. Brightness stays at current level

### Bathroom Control (Strip 3)

**AUTO Mode (Default):**

1. Set mode to AUTO via frontend or MQTT
2. Motion detected by PIR sensor (Pin 2)
3. Strip 3 automatically turns on
4. Timer resets on each motion detection
5. Automatically turns off after 60 seconds of no motion

**Manual Control:**

1. Set mode to ON via frontend or MQTT - strip stays on
2. Set mode to OFF via frontend or MQTT - strip stays off
3. In ON/OFF modes, motion sensor is completely ignored
4. Dimming works in ON/OFF modes (hold Button 4)

### Bedroom Control (Strip 4)

1. Press Button 4 (Pin 13) to toggle on/off
2. Hold Button 4 for dimming
3. Full feature parity with Strip 1 (transitions, dimming, blink)

### Floor Lighting Control

1. Press Button 3 (Pin 27)
2. Relay (Pin 26) toggles ON/OFF
3. Controls floor lighting circuit with standard LED diodes
4. Simple toggle - no dimming or transitions

## Notes

- Strip 2 (Kitchen extension) cannot be controlled independently
- Strip 3 (Bathroom) has three modes: OFF, AUTO, ON
  - In AUTO mode: motion sensor controls the strip
  - In OFF/ON modes: motion sensor is ignored, manual control only
- Strip 4 (Bedroom) uses RGBW type with GRBW protocol
- All strips use white color with RGB+W channels
- Strip 4 uses special warm white configuration for comfortable bedroom lighting
- Transitions are randomly selected for visual variety
- System initializes all strips to OFF state
- Floor lighting (relay circuit) is independent from LED strips
- Floor lighting uses standard LED diodes, not LED strips
- Floor lighting has simple toggle control (no dimming)
- System works offline - local controls (buttons, PIR) function without WiFi/MQTT
- MQTT buffer size increased to 1024 bytes to support unified status messages

## Version Information

- Module: LED Controller
- Last Updated: 2024
- Hardware: ESP32
- LED Types: WS2815 RGBW (Strips 0-3), WS2812 RGBW GRBW (Strip 4)
- Number of Strips: 5
- Number of Buttons: 4
- Number of Relays: 1
