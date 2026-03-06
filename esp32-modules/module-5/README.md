# Module-5: Appliance Controller

ESP32 module for controlling five appliances via relays: Audio System, Water Pump, Refrigerator, WC Fan, and Boiler.

## Hardware Configuration

### Pin Assignment

| Component                    | Pin | Type            | Description                    |
| ---------------------------- | --- | --------------- | ------------------------------ |
| **Relay 0** (Audio System)   | 14  | Output          | Relay control for audio        |
| **Relay 1** (Water Pump)     | 15  | Output          | Relay control for pump         |
| **Relay 2** (Refrigerator)   | 16  | Output          | Relay control for refrigerator |
| **Relay 3** (WC Fan)         | 23  | Output          | Relay control for WC fan       |
| **Relay 4** (Boiler)         | 27  | Output          | Relay control for boiler       |
| **Button 0** (Audio System)  | 17  | Input (Pull-up) | Manual toggle for relay 0       |
| **Button 1** (Water Pump)   | 21  | Input (Pull-up) | Manual toggle for relay 1       |
| **Button 2** (Refrigerator)  | 22  | Input (Pull-up) | Manual toggle for relay 2       |
| **Button 3** (WC Fan)         | 25  | Input (Pull-up) | Manual toggle for relay 3       |
| **Button 4** (Boiler)        | 26  | Input (Pull-up) | Manual toggle for relay 4       |

### Appliance Details

| Relay | Name          | Relay Pin | Button Pin | Description                |
| ----- | ------------- | --------- | ----------- | -------------------------- |
| **0** | Audio System  | 14        | 17          | Audio system control       |
| **1** | Water Pump    | 15        | 21          | Water pump control         |
| **2** | Refrigerator  | 16        | 22          | Refrigerator control       |
| **3** | WC Fan        | 23        | 25          | WC fan control             |
| **4** | Boiler        | 27        | 26          | Boiler control             |

### Button Functions

- **Click**: Toggle relay ON/OFF
- Simple toggle behavior (no dimming or hold)

## Network Configuration

- **WiFi SSID**: `SmartCamper`
- **WiFi Password**: `12344321`
- **MQTT Broker IP**: `192.168.4.1` (Raspberry Pi)
- **MQTT Broker Port**: `1883`
- **Module ID**: `module-5`
- **MQTT Buffer Size**: 1024 bytes

## MQTT Topics

### Published (Status)

| Topic                                 | Message Format                        | Update Frequency                                             |
| ------------------------------------- | ------------------------------------- | ------------------------------------------------------------ |
| `smartcamper/sensors/module-5/status` | `{"relays": {"0": {"state": "ON"}, ...}}` | On change only (button press, MQTT command, force_update) |
| `smartcamper/heartbeat/module-5`      | `{"timestamp": ..., "moduleId": "module-5", "uptime": ..., "wifiRSSI": ...}` | Every 10 seconds                                               |

### Subscribed (Commands)

| Topic                                           | Payload | Action                          |
| ----------------------------------------------- | ------- | ------------------------------- |
| `smartcamper/commands/module-5/relay/{index}/toggle` | `{}`    | Toggle relay                 |
| `smartcamper/commands/module-5/force_update`    | `{}`    | Force status update |

### Command Format

- `{index}` can be 0, 1, 2, 3, or 4 (for the five relays)
- Example: `smartcamper/commands/module-5/relay/0/toggle` - toggles relay 0 (Audio System)
- Example: `smartcamper/commands/module-5/relay/3/toggle` - toggles relay 3 (WC Fan)
- Example: `smartcamper/commands/module-5/relay/4/toggle` - toggles relay 4 (Boiler)

## Features

### Relay Control

- Each relay can be controlled independently
- Physical buttons work offline (without WiFi/MQTT)
- MQTT commands work when module is connected
- Status is published automatically on every change

### Behavior

- Same behavior as Ambient relay in module-2
- Simple toggle - no dimming or complex modes
- Button toggles relay immediately on press

## Installation and Setup

1. **Upload firmware:**

   ```bash
   cd esp32-modules/module-5
   pio run --target upload
   ```

2. **Monitor serial output:**

   ```bash
   pio device monitor
   ```

   - Check WiFi/MQTT connection
   - Monitor button presses
   - Check relay status

3. **Test functionality:**
   - Test physical buttons
   - Check frontend dashboard
   - Test MQTT commands

## Troubleshooting

### Relays not working

- Check relay power supply (12V DC)
- Check relay connections (pins 14, 15, 16, 23, 27)
- Check serial output for errors
- Ensure proper ground connection between ESP32 and relays

### Buttons not working

- Check button connections (pins 17, 21, 22, 25, 26)
- Check pull-up resistors (internal pull-ups are used)
- Test with multimeter (button should connect pin to GND when pressed)
- Check serial output for press messages

### No MQTT connection

- Check MQTT broker IP address in `src/Config.h`
- Check WiFi connection status
- Ensure MQTT broker is running on Raspberry Pi
- Check serial output for MQTT errors

## Offline Operation

Module works independently:

- Physical buttons function without WiFi/MQTT
- Relays can be controlled via buttons offline
- Status updates are only published when connected
- No errors when offline - all local functions work

## Electrical Specifications

- **Relays**: Standard 12V relay module (NO/NC contacts)
- **Current consumption**: Depends on appliances (audio, pump, refrigerator)
- **Power supply**: 12V DC (sufficient capacity for all appliances)
- **Buttons**: Momentary push buttons (normally open)

## Architecture

- **ModuleManager**: WiFi, MQTT, Heartbeat, Commands
- **ApplianceManager**: Coordinates all appliance functionality
- **RelayController**: Manages relays (control, status)
- **ButtonHandler**: Processes button inputs (debouncing, state machine)

## Serial Debug Output

Enable/disable in `src/Config.h`:

- `DEBUG_SERIAL`: Serial console output
- `DEBUG_MQTT`: MQTT messages for publish/subscribe
- `DEBUG_VERBOSE`: Detailed relay state changes
