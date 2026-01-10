# Module-4: Heating Control - Damper & Table Management

ESP32 module for controlling heating system dampers (air vents) with servo motors and table lift control.

## Overview

Module 4 controls:

- **Air distribution dampers** using servo motors (SG90). Each damper can be positioned at three angles: 0° (closed), 45° (half-open), or 90° (open). Dampers can be controlled via MQTT commands from the backend or via physical toggle buttons.
- **Table lift** using two relays (up/down) and physical buttons. Table can be controlled via MQTT commands or physical buttons with hold-to-move and double-click auto-movement modes.

## Hardware Configuration

### Pin Assignment

| Component             | Pin | Type   | Description                                                   |
| --------------------- | --- | ------ | ------------------------------------------------------------- |
| **Servo Motor 0**     | 2   | Output | SG90 servo signal for Damper 0 - Front (Yellow/Orange wire)   |
| **Button 0**          | 4   | Input  | Toggle button for Damper 0 - Front (INPUT_PULLUP)             |
| **Servo Motor 1**     | 5   | Output | SG90 servo signal for Damper 1 - Rear (Yellow/Orange wire)    |
| **Button 1**          | 16  | Input  | Toggle button for Damper 1 - Rear (INPUT_PULLUP)              |
| **Servo Motor 2**     | 12  | Output | SG90 servo signal for Damper 2 - Bath (Yellow/Orange wire)    |
| **Button 2**          | 17  | Input  | Toggle button for Damper 2 - Bath (INPUT_PULLUP)              |
| **Servo Motor 3**     | 13  | Output | SG90 servo signal for Damper 3 - Shoes (Yellow/Orange wire)   |
| **Button 3**          | 18  | Input  | Toggle button for Damper 3 - Shoes (INPUT_PULLUP)             |
| **Servo Motor 4**     | 14  | Output | SG90 servo signal for Damper 4 - Cockpit (Yellow/Orange wire) |
| **Button 4**          | 19  | Input  | Toggle button for Damper 4 - Cockpit (INPUT_PULLUP)           |
| **Table Relay Up**    | 21  | Output | Relay for table lift up movement (NO contact -> +)            |
| **Table Relay Down**  | 22  | Output | Relay for table lift down movement (NO contact -> +)          |
| **Table Button Up**   | 23  | Input  | Button for table up movement (INPUT_PULLUP)                   |
| **Table Button Down** | 25  | Input  | Button for table down movement (INPUT_PULLUP)                 |

### Damper Names

| Damper Index | Name    | Location     |
| ------------ | ------- | ------------ |
| 0            | Front   | Front area   |
| 1            | Rear    | Rear area    |
| 2            | Bath    | Bathroom     |
| 3            | Shoes   | Shoe cabinet |
| 4            | Cockpit | Driver cabin |

### Servo Motor Wiring (SG90)

- **Red wire** → 5V on ESP32
- **Brown/Black wire** → GND on ESP32
- **Yellow/Orange wire** → GPIO pin (signal)

### Button Wiring

- One terminal → GPIO pin
- Other terminal → GND
- Internal pull-up resistor enabled (INPUT_PULLUP)

## Architecture

- **ModuleManager**: Handles common infrastructure (Network, MQTT, Heartbeat, Commands)
- **DamperManager**: Coordinates all damper controllers
- **DamperController**: Controls a single damper (servo + button)
- **ServoController**: Reusable servo motor control class
- **TableManager**: Coordinates table lift controller
- **TableController**: Controls table lift (relays + buttons)
- **SensorManager**: Coordinates all components
- **CommandHandler**: Handles MQTT commands from backend
- **NetworkManager**: WiFi connection management
- **MQTTManager**: MQTT broker communication
- **HeartbeatManager**: Module status reporting

## Configuration

Edit `src/Config.h` to configure:

- WiFi credentials
- MQTT broker IP and port
- Number of dampers: `NUM_DAMPERS` (currently 5)
- Servo pins: `DAMPER_0_SERVO_PIN` through `DAMPER_4_SERVO_PIN`
- Button pins: `DAMPER_0_BUTTON_PIN` through `DAMPER_4_BUTTON_PIN`
- Table relay pins: `TABLE_RELAY_UP_PIN` (21), `TABLE_RELAY_DOWN_PIN` (22)
- Table button pins: `TABLE_BUTTON_UP_PIN` (23), `TABLE_BUTTON_DOWN_PIN` (25)
- Table timing: `TABLE_DOUBLE_CLICK_TIMEOUT` (500ms), `TABLE_AUTO_MOVE_DURATION` (5000ms), `TABLE_START_DELAY` (600ms)

## Building and Uploading

1. Install PlatformIO
2. Open the project in PlatformIO
3. Build: `pio run`
4. Upload: `pio run --target upload`
5. Monitor: `pio device monitor`

## MQTT Topics

### Published (Status)

| Topic                                               | Message Format                         | Update Frequency                                  |
| --------------------------------------------------- | -------------------------------------- | ------------------------------------------------- |
| `smartcamper/sensors/module-4/damper/{index}/angle` | `{"angle": 45}`                        | On change (when servo reaches target)             |
| `smartcamper/sensors/module-4/table/direction`      | `{"direction":"up","autoMoving":true}` | On change (direction: "up", "down", or "stopped") |

### Subscribed (Commands)

| Topic                                                    | Payload                                                       | Action                                                                    |
| -------------------------------------------------------- | ------------------------------------------------------------- | ------------------------------------------------------------------------- |
| `smartcamper/commands/module-4/damper/{index}/set_angle` | `{"type":"damper","index":0,"action":"set_angle","angle":45}` | Set damper angle (0-180°). Index: 0-4 (Front, Rear, Bath, Shoes, Cockpit) |
| `smartcamper/commands/module-4/table/move_up`            | `{"type":"table","action":"move_up"}`                         | Start manual upward movement (hold-to-move)                               |
| `smartcamper/commands/module-4/table/move_down`          | `{"type":"table","action":"move_down"}`                       | Start manual downward movement (hold-to-move)                             |
| `smartcamper/commands/module-4/table/stop`               | `{"type":"table","action":"stop"}`                            | Stop table movement                                                       |
| `smartcamper/commands/module-4/table/move_up_auto`       | `{"type":"table","action":"move_up_auto","duration":5000}`    | Auto move up for duration (default 5000ms)                                |
| `smartcamper/commands/module-4/table/move_down_auto`     | `{"type":"table","action":"move_down_auto","duration":5000}`  | Auto move down for duration (default 5000ms)                              |
| `smartcamper/commands/module-4/force_update`             | `{}`                                                          | Force publish all damper and table statuses                               |

### Heartbeat

| Topic                            | Message Format                                                                | Update Frequency |
| -------------------------------- | ----------------------------------------------------------------------------- | ---------------- |
| `smartcamper/heartbeat/module-4` | `{"timestamp":1234567890,"moduleId":"module-4","uptime":3600,"wifiRSSI":-65}` | Every 10 seconds |

## Operation

### Button Control

- **Click**: Cycles through positions: 0° → 45° → 90° → 0°
- **Debouncing**: 50ms delay to prevent multiple triggers
- **Offline operation**: Button works even without WiFi/MQTT connection

### Servo Movement

- **Smooth movement**: 2-3 seconds for 0-90° rotation
- **Step size**: 5° per step
- **Step delay**: 150ms between steps
- **Angle range**: 0-180° (validated)

### Position States

- **0° (Closed)**: Damper blade vertical - blocks air flow
- **45° (Half-open)**: Damper blade at 45° - partial air flow
- **90° (Open)**: Damper blade horizontal - full air flow

## Table Lift Control

### Physical Button Control

- **Hold button**: Manual movement - table moves while button is held, stops when released
- **Double-click**: Auto movement - double-click within 500ms triggers automatic movement for 5 seconds
- **Safety**: Pressing any button during auto movement immediately stops it
- **Debouncing**: 50ms delay to prevent multiple triggers
- **Start delay**: 600ms delay before starting movement (allows double-click detection)
- **Offline operation**: Buttons work even without WiFi/MQTT connection

### Frontend Control

- **Single click/touch**: Triggers automatic movement (5 seconds) - simplified interface for touch screens
- **During auto movement**: Clicking any button stops the movement immediately
- **Visual feedback**: Button shows bright blue glow when table is moving

### MQTT Commands

- **move_up**: Manual upward movement (hold-to-move mode)
- **move_down**: Manual downward movement (hold-to-move mode)
- **stop**: Stop any movement
- **move_up_auto**: Automatic upward movement for specified duration (default 5000ms)
- **move_down_auto**: Automatic downward movement for specified duration (default 5000ms)

### Safety Features

- **Mutual exclusion**: Up and down relays cannot be active simultaneously
- **Auto-stop protection**: Any button press during auto movement stops it immediately
- **Command validation**: Invalid commands are ignored

## Installation & Setup

1. **Upload firmware:**

   ```bash
   cd esp32-modules/module-4
   pio run --target upload
   ```

2. **Monitor serial output:**

   ```bash
   pio device monitor
   ```

   - Check WiFi connection status
   - Verify MQTT connection
   - Monitor damper status

3. **Verify operation:**
   - Check frontend dashboard for damper control
   - Verify heartbeat status icon (blue = online, red = offline)
   - Test button control (works offline)

## Troubleshooting

### No WiFi Connection

- Check SSID and password in `src/Config.h`
- Verify WiFi router is broadcasting `SmartCamper` network
- Check serial output for connection errors

### No MQTT Connection

- Verify MQTT broker IP address in `src/Config.h` (default: `192.168.4.1`)
- Check if Raspberry Pi is running and MQTT broker is active
- Verify network connectivity (ping MQTT broker IP)

### Servo Not Moving

- Check wiring (VCC, GND, Signal pin 2)
- Verify servo is powered (5V)
- Check serial output for servo errors
- Verify angle is valid (0-180°)

### Button Not Working

- Check button wiring (GPIO 4 to button, button to GND)
- Verify button is not stuck
- Check serial output for button state changes
- Button works offline - no network required

## Offline Operation

Module operates independently:

- Servo and button work even without WiFi/MQTT
- Status is published when connection is restored
- Auto-reconnect to WiFi and MQTT every 2-3 seconds

## Status

✅ Base infrastructure complete
✅ WiFi connection management
✅ MQTT communication
✅ Heartbeat system
✅ Command handling
✅ Damper control with servo motors
✅ Button control (offline operation)
✅ Smooth servo movement
✅ Status publishing
✅ Table lift control with relays
✅ Table button control (hold-to-move + double-click auto)
✅ Table MQTT commands
✅ Table status publishing
