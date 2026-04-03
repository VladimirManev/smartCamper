# Module-2: LED Strip Controller

ESP32 module for controlling multiple LED strips with buttons, motion sensor, dimming, and transitions.

## Hardware Configuration

### Pin Assignment

| Component | Pin | Type | Description |
|-----------|-----|------|-------------|
| **Strip 0** (Kitchen main) | 33 | Output | 44 LEDs, RGBW |
| **Strip 1** (Main lighting) | 18 | Output | 178 LEDs, RGBW |
| **Strip 2** (Kitchen extension) | 19 | Output | 23 LEDs, RGBW (synced with Strip 0) |
| **Strip 3** (Bathroom) | 25 | Output | 53 LEDs, RGBW (motion activated) |
| **Strip 4** (Bedroom) | 5 | Output | 30 LEDs, RGBW (GRBW protocol) |
| **Button 1** | 4 | Input (Pull-up) | Kitchen control (Strips 0+2) |
| **Button 2** | 12 | Input (Pull-up) | Main lighting (Strip 1) |
| **Button 3** | 27 | Input (Pull-up) | Floor relay toggle |
| **Button 4** | 13 | Input (Pull-up) | Bedroom control (Strip 4) |
| **Relay 0** | 26 | Output | Floor lighting relay |
| **Power Relay** | 32 | Output | LED strips power management (10A relay) |
| **PIR Sensor** | 2 | Input | Motion detection (Strip 3) |

### LED Strip Details

| Strip | LEDs | Type | Protocol | Control |
|-------|------|------|----------|---------|
| **0** | 44 | WS2815 RGBW | RGBW | Button 1 |
| **1** | 178 | WS2815 RGBW | RGBW | Button 2 |
| **2** | 23 | WS2815 RGBW | RGBW | Auto (synced with Strip 0) |
| **3** | 53 | WS2815 RGBW | RGBW | PIR sensor + MQTT |
| **4** | 30 | WS2812 RGBW | GRBW | Button 4 |

### Button Functions

- **Click**: Toggle strip ON/OFF (with random transition)
- **Hold** (>250ms): Start dimming (brightness up/down alternates)
- **Button 3**: Simple relay toggle (no dimming)

### Strip 3 (Bathroom) Modes

- **OFF**: Strip stays off
- **AUTO**: Motion sensor controls strip (60s timeout)
- **ON**: Strip stays on

**Strip 3 brightness (MQTT / app):** There is **no** physical button hold-dimming on this strip, but `strip/3/brightness` uses the same **smooth transition** as the other strips whenever the strip is **ON** (including **AUTO** while the light is already on from PIR). When a smooth MQTT brightness change **finishes** in **AUTO** mode, the firmware updates `lastAutoBrightness` so the next motion-triggered ON uses that level.

**Status publish after `brightness`:** Full JSON on `smartcamper/sensors/module-2/status` is sent when the **smooth transition completes**, not at command receive (avoids publishing stale `brightness` while fading).

## Network Configuration

- **WiFi SSID**: `SmartCamper`
- **WiFi Password**: `12344321`
- **MQTT Broker IP**: `192.168.4.1` (Raspberry Pi)
- **MQTT Broker Port**: `1883`
- **Module ID**: `module-2`
- **MQTT Buffer Size**: 1024 bytes

## MQTT Topics

### Published (Status)

| Topic | Message Format | Update Frequency |
|-------|---------------|------------------|
| `smartcamper/sensors/module-2/status` | `{"strips": {...}, "relays": {...}}` | On change (after transitions where applicable; MQTT `brightness` after fade completes) |
| `smartcamper/heartbeat/module-2` | `{"timestamp": ..., "moduleId": "module-2", "uptime": ..., "wifiRSSI": ...}` | Every 10 seconds |

### Subscribed (Commands)

| Topic | Payload | Action |
|-------|---------|--------|
| `smartcamper/commands/module-2/strip/{index}/on` | `{}` | Turn on strip |
| `smartcamper/commands/module-2/strip/{index}/off` | `{}` | Turn off strip |
| `smartcamper/commands/module-2/strip/{index}/toggle` | `{}` | Toggle strip |
| `smartcamper/commands/module-2/strip/{index}/brightness` | `{"value": 1-255}` | Smooth brightness (clamped by firmware) |
| `smartcamper/commands/module-2/strip/3/mode` | `{"mode": "OFF"\|"AUTO"\|"ON"}` | Set Strip 3 mode |
| `smartcamper/commands/module-2/relay/toggle` | `{}` | Toggle relay |
| `smartcamper/commands/module-2/force_update` | `{}` | Force status update |

## Features

### Power Relay Management

The module includes a power relay (GPIO 32 — see `Config.h`) that physically cuts power to all LED strips when they are all turned off. This prevents standby power consumption (0.5A+) from WS2812 controllers.

**How it works:**
- Relay turns ON automatically when any strip is turned on (with 100ms stabilization delay)
- Relay turns OFF automatically when all strips are off (safety check every 60 seconds)
- Prevents battery drain when all strips are off

**Safety check:**
- Every 60 seconds, the system verifies all strips are off
- If relay is ON and all strips are off, relay is automatically turned off
- This ensures relay never stays ON unnecessarily

### Transitions
- 8 different transition effects (4 for ON, 4 for OFF)
- Randomly selected on each toggle
- Duration: 1 second

### Dimming
- **Button hold** (strips 0, 1, 4 only): Hold >250ms to start; speed ~50 brightness units/s; alternates up/down; blink at max. Strip 3 has **no** dimmer button.
- **MQTT `brightness`**: All strips **including strip 3** — smooth fade to target; final status published when the fade **completes**.

### Motion Sensor
- PIR sensor on pin 2
- Only active when Strip 3 is in AUTO mode
- 60-second timeout after last motion
- Auto turn-on on motion detection

## Installation & Setup

1. **Upload firmware:**
   ```bash
   cd esp32-modules/module-2
   pio run --target upload
   ```

2. **Monitor serial output:**
   ```bash
   pio device monitor
   ```
   - Check WiFi/MQTT connection
   - Monitor button presses
   - Check LED strip status

3. **Verify operation:**
   - Test physical buttons
   - Check frontend dashboard
   - Verify motion sensor (Strip 3 in AUTO mode)

## Troubleshooting

### No LED Response
- Check power supply (12V for WS2815 strips)
- Verify data pin connections (33, 18, 19, 25, 5)
- Check serial output for errors
- Ensure proper ground connection between ESP32 and LED strips

### Buttons Not Working
- Check button wiring (pins 4, 12, 27, 13)
- Verify pull-up resistors (internal pull-ups used)
- Test with multimeter (button should connect pin to GND when pressed)
- Check serial output for button press messages

### Motion Sensor Not Working
- Verify Strip 3 is in AUTO mode (check frontend or MQTT)
- Check PIR sensor wiring (pin 2, VCC, GND)
- Adjust PIR sensitivity potentiometer (if available)
- Check serial output for motion detection messages

### Dimming Not Working
- **Button dimming:** Strip must be ON; hold **>250ms** (strips 0, 1, 4 only — not bathroom).
- **App / MQTT dimming:** Works for **all strips including bathroom** when the strip is ON; if the UI lags, confirm status topic updates after the fade ends.
- Check serial output for dimming / smooth brightness messages

### No MQTT Connection
- Verify MQTT broker IP in `src/Config.h`
- Check WiFi connection status
- Verify MQTT broker is running on Raspberry Pi
- Check serial output for MQTT errors

## Offline Operation

Module works independently:
- Physical buttons function without WiFi/MQTT
- LED strips can be controlled via buttons offline
- PIR sensor works offline (Strip 3 in AUTO mode)
- Status updates are only published when connected
- No errors when offline - all local functions work

## Electrical Specifications

- **LED Strips**: WS2815/WS2812 RGBW, 12V
- **Current Consumption**: Varies by strip (calculate based on LEDs and brightness)
- **Power Supply**: 12V DC (sufficient capacity for all strips)
- **Relay**: Standard 12V relay module (NO/NC contacts)
- **Buttons**: Momentary push buttons (normally open)

## Architecture

- **ModuleManager**: WiFi, MQTT, Heartbeat, Commands
- **LEDManager**: Coordinates all LED functionality
- **LEDStripController**: Manages LED strips (control, transitions, dimming)
- **ButtonHandler**: Processes button inputs (debouncing, state machine)
- **RelayController**: Controls relay for floor lighting
- **PIRSensorHandler**: Motion detection for Strip 3

## Serial Debug Output

Enable/disable in `src/Config.h`:
- `DEBUG_SERIAL`: Serial console output
- `DEBUG_MQTT`: MQTT publish/subscribe messages
- `DEBUG_VERBOSE`: Detailed LED strip state changes

