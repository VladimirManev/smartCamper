# Module-3: Floor Heating Controller

ESP32 module for controlling floor heating system with automatic temperature-based control, manual override buttons, and 4 independent heating circles.

## Hardware Configuration

### Pin Assignment

| Component | Pin | Type | Description |
|-----------|-----|------|-------------|
| **Circle 0 Temp Sensor** (Central 1) | 25 | Input | DS18B20 temperature sensor (OneWire) |
| **Circle 1 Temp Sensor** (Central 2) | 26 | Input | DS18B20 temperature sensor (OneWire) |
| **Circle 2 Temp Sensor** (Bathroom) | 27 | Input | DS18B20 temperature sensor (OneWire) |
| **Circle 3 Temp Sensor** (Podium) | 33 | Input | DS18B20 temperature sensor (OneWire) |
| **Circle 0 Relay** (Central 1) | 4 | Output | Relay control for heating circle 0 |
| **Circle 1 Relay** (Central 2) | 5 | Output | Relay control for heating circle 1 |
| **Circle 2 Relay** (Bathroom) | 18 | Output | Relay control for heating circle 2 |
| **Circle 3 Relay** (Podium) | 19 | Output | Relay control for heating circle 3 |
| **Button 0** (Central 1) | 12 | Input (Pull-up) | Manual toggle for circle 0 |
| **Button 1** (Central 2) | 13 | Input (Pull-up) | Manual toggle for circle 1 |
| **Button 2** (Bathroom) | 14 | Input (Pull-up) | Manual toggle for circle 2 |
| **Button 3** (Podium) | 15 | Input (Pull-up) | Manual toggle for circle 3 |

### Heating Circle Details

| Circle | Name | Temperature Sensor Pin | Relay Pin | Button Pin |
|--------|------|------------------------|-----------|------------|
| **0** | Central 1 | 25 | 4 | 12 |
| **1** | Central 2 | 26 | 5 | 13 |
| **2** | Bathroom | 27 | 18 | 14 |
| **3** | Podium | 33 | 19 | 15 |

### Temperature Sensor Specifications

**DS18B20 (All Circles):**
- Temperature range: -55°C to 125°C
- Accuracy: ±0.5°C
- Resolution: 12-bit (0.0625°C precision)
- Update interval: 1 second (5-second average)
- OneWire protocol with 4.7kΩ pull-up resistor required

### Temperature Control Settings

- **Target Temperature**: 33°C (default, configurable in future)
- **Hysteresis**: 2°C
- **Turn OFF**: When temperature reaches 33°C
- **Turn ON**: When temperature drops below 32°C
- **Measurement Interval**: 30 seconds (automatic control check)
- **Temperature Reading**: Every 1 second (averaged over 5 seconds)

## Network Configuration

- **WiFi SSID**: `SmartCamper`
- **WiFi Password**: `12344321`
- **MQTT Broker IP**: `192.168.4.1` (Raspberry Pi)
- **MQTT Broker Port**: `1883`
- **Module ID**: `module-3`

## MQTT Topics

### Published (Status & Sensor Data)

| Topic | Message Format | Update Frequency |
|-------|---------------|------------------|
| `smartcamper/sensors/module-3/status` | `{"type": "full", "data": {"circles": {...}}}` or `{"type": "circle", "index": 0, "mode": "TEMP_CONTROL", "relay": "ON", "temperature": 32, "error": false}` | On change (button press, MQTT command, automatic control change, force_update) |
| `smartcamper/errors/module-3/circle/{index}` | `{"error": true, "type": "sensor_disconnected", "message": "Temperature sensor disconnected", "timestamp": 1234567890}` | Once when error occurs |
| `smartcamper/heartbeat/module-3` | `{"timestamp": 1234567890, "moduleId": "module-3", "uptime": 3600, "wifiRSSI": -65}` | Every 10 seconds |

### Subscribed (Commands)

| Topic | Payload | Action |
|-------|---------|--------|
| `smartcamper/commands/module-3/circle/{index}/on` | `{}` | Enable TEMP_CONTROL mode (temperature-based control) |
| `smartcamper/commands/module-3/circle/{index}/off` | `{}` | Disable circle (OFF mode) |
| `smartcamper/commands/module-3/force_update` | `{}` | Force status update |

## Features

### Circle Modes

**OFF Mode:**
- Circle is completely disabled
- No temperature measurement
- Relay is off
- Button toggles to TEMP_CONTROL mode

**TEMP_CONTROL Mode:**
- Circle is in temperature-based automatic control
- Temperature is measured continuously using async non-blocking method (every 1 second, averaged over 5 seconds)
- Temperature conversion takes ~800ms (non-blocking, doesn't block button handling)
- Automatic relay control based on temperature:
  - Relay ON when temperature < 32°C
  - Relay OFF when temperature >= 33°C
- Relay turns on immediately after first temperature reading (~1 second after enabling circle)
- Hysteresis prevents rapid cycling (2°C difference)
- Works completely offline (no network required)
- Control check every 30 seconds (or immediately when new temperature is available)
- Button toggles to OFF mode

### Error Handling

- If sensor fails 3 consecutive readings (3 failed attempts, ~2.4 seconds total), circle is automatically disabled (OFF mode)
- Error is published to `smartcamper/errors/module-3/circle/{index}` topic (published once when error occurs)
- If sensor is not found during initialization, error is reported immediately
- When sensor recovers, normal status is published and circle automatically returns to previous state (TEMP_CONTROL if it was enabled)

### Offline Operation

- All functions work without WiFi/MQTT connection
- Automatic temperature control continues offline
- Physical buttons work offline
- Status updates are only published when connected

## Installation & Setup

1. **Upload firmware:**
   ```bash
   cd esp32-modules/module-3
   pio run --target upload
   ```

2. **Monitor serial output:**
   ```bash
   pio device monitor
   ```
   - Check WiFi/MQTT connection
   - Monitor temperature readings
   - Check relay state changes
   - Monitor button presses

3. **Verify operation:**
   - Check frontend dashboard for circle status
   - Test physical buttons (should toggle circles)
   - Verify automatic control (watch temperature and relay state)
   - Check heartbeat status icon (blue = online, red = offline)

## Troubleshooting

### No WiFi Connection
- Check SSID and password in `src/Config.h`
- Verify WiFi router is broadcasting `SmartCamper` network
- Check serial output for connection errors

### No MQTT Connection
- Verify MQTT broker IP address in `src/Config.h` (default: `192.168.4.1`)
- Check if Raspberry Pi is running and MQTT broker is active
- Verify network connectivity (ping MQTT broker IP)

### Temperature Sensor Not Working
- **DS18B20**: Verify OneWire pull-up resistor (4.7kΩ) on each sensor pin
- Check sensor wiring (VCC, GND, DATA pin)
- Verify sensor is properly sealed and positioned
- Check serial output for sensor errors (look for "Failed to read temperature")
- Test sensor with multimeter (should read ~3.3V on DATA pin when idle)

### Relay Not Switching
- Check relay wiring (relay pins: 4, 5, 18, 19)
- Verify relay module power supply (12V for relay coil)
- Test relay with multimeter (should show continuity when ON)
- Check serial output for relay state changes
- Verify relay module is properly connected (IN pin to ESP32, VCC/GND to power)

### Button Not Working
- Check button wiring (button pins: 12, 13, 14, 15)
- Verify pull-up resistors (internal pull-ups used)
- Test with multimeter (button should connect pin to GND when pressed)
- Check serial output for button press messages
- Verify button is momentary push button (normally open)
- Button debouncing: 100ms debounce delay, 300ms minimum interval between presses
- If button is not responsive, check for blocking operations in code (should be none)

### Automatic Control Not Working
- Verify temperature sensor is reading correctly (check serial output)
- Check if circle is in manual override mode (button was pressed)
- Verify temperature thresholds in `src/Config.h`:
  - `HEATING_TURN_OFF_TEMP`: 33°C
  - `HEATING_TURN_ON_TEMP`: 32°C
- Check serial output for automatic control messages
- Ensure measurement interval is correct (30 seconds)

### Circle Stays ON/OFF
- Check circle mode (OFF or TEMP_CONTROL) in serial output
- Verify temperature sensor reading (should be valid, not 0 or NaN)
- Check relay state in serial output
- Check if sensor has error (3 failed readings will disable circle)
- Try enabling TEMP_CONTROL mode via MQTT: `smartcamper/commands/module-3/circle/{index}/on`

### Wrong Temperature Reading
- Verify DS18B20 sensor is properly sealed and positioned
- Check for loose connections on sensor DATA pin
- Verify pull-up resistor (4.7kΩ) is connected between DATA and VCC
- Check sensor is not damaged (test with another sensor)
- Verify sensor address (if multiple sensors on same OneWire bus)

## Offline Operation

Module operates completely independently:
- Automatic temperature control works without WiFi/MQTT
- Physical buttons function offline
- Temperature sensors continue reading offline
- Status updates are only published when connection is restored
- No errors when offline - all local functions work normally

## Electrical Specifications

- **Temperature Sensors**: DS18B20, 3.3V-5V, OneWire protocol
- **Pull-up Resistor**: 4.7kΩ (required for each DS18B20 sensor)
- **Relays**: Standard 12V relay modules (NO/NC contacts)
- **Relay Coil**: 12V DC (check relay module specifications)
- **Buttons**: Momentary push buttons (normally open)
- **Power Supply**: 5V for ESP32, 12V for relay coils

## Architecture

- **ModuleManager**: Handles WiFi, MQTT, Heartbeat, Commands
- **FloorHeatingManager**: Coordinates all floor heating functionality
- **FloorHeatingController**: Manages relay control and automatic temperature control
- **FloorHeatingSensor**: Temperature sensor reading and averaging (DS18B20) - uses async non-blocking state machine
- **FloorHeatingButtonHandler**: Processes button inputs (debouncing, toggle) - non-blocking operation

### Performance Optimizations

- **Non-blocking temperature reading**: Uses async state machine (conversion starts, then reads after ~800ms)
- **Immediate relay control**: Relay turns on immediately after first temperature reading (~1 second after enabling circle)
- **No blocking delays**: All operations are non-blocking to ensure responsive button handling
- **Optimized debouncing**: 100ms debounce delay with 300ms minimum interval between presses

## Serial Debug Output

Enable/disable in `src/Config.h`:
- `DEBUG_SERIAL`: Serial console output
- `DEBUG_MQTT`: MQTT publish/subscribe messages
- `DEBUG_VERBOSE`: Detailed sensor readings and control decisions

## Future Features (Prepared in Code)

- Configurable target temperature (currently fixed at 33°C)
- Temperature adjustment via MQTT commands
- Schedule-based control
- Energy consumption monitoring

