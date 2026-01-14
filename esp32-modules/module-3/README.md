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
| **Leveling Sensor SDA** (MPU6050) | 21 | Input/Output | I²C SDA for leveling sensor (GY-521) |
| **Leveling Sensor SCL** (MPU6050) | 22 | Input/Output | I²C SCL for leveling sensor (GY-521) |
| **BOOT Button** (Zeroing) | 0 | Input (Pull-up) | Long press (3s) to zero leveling reference |
| **Built-in LED** (Feedback) | 2 | Output | Visual feedback for zeroing (3 quick blinks) |

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

## Leveling Sensor (MPU6050 / GY-521)

### Hardware Configuration

**Sensor:** GY-521 board with MPU6050 (6-DOF IMU)
- **I²C SDA:** GPIO 21
- **I²C SCL:** GPIO 22
- **Power:** 3.3V (VCC), GND
- **Communication:** I²C protocol

**Zeroing Function:**
- **Button:** BOOT button (GPIO 0)
- **Action:** Long press for 3 seconds to save current position as zero reference
- **Feedback:** Built-in LED (GPIO 2) blinks 3 times + serial message
- **Storage:** Zero offsets stored in ESP32 Preferences (non-volatile flash)

### Sensor Specifications

- **Type:** MPU6050 (6-DOF: 3-axis accelerometer + 3-axis gyroscope)
- **Range:** ±16g (accelerometer), ±2000°/s (gyroscope)
- **Accuracy:** ±0.5° for leveling applications
- **Calibration:** Gyroscope only (accelerometer uses gravity as absolute reference)
- **Update Rate:** 0.5 seconds when active (on-demand)

### Operation Mode

**On-Demand Data Streaming:**
- Sensor only measures and publishes when actively requested
- Frontend sends `start` command every 10 seconds (keep-alive)
- ESP32 publishes data every 0.5 seconds for 22 seconds after last `start` command
- Automatically stops measuring and publishing after timeout
- Saves power and reduces MQTT traffic when not in use

### Zeroing Procedure

1. Park camper in desired level position
2. Ensure sensor is firmly mounted
3. Press and hold BOOT button (GPIO 0) for 3 seconds
4. LED blinks 3 times (visual confirmation)
5. Serial message confirms zeroing
6. Current pitch and roll angles saved as zero offsets in flash memory
7. On next boot, sensor uses saved offsets as reference

### Angle Measurement

- **Pitch (X-axis):** Forward/backward tilt (positive = forward tilt)
- **Roll (Y-axis):** Left/right tilt (positive = right tilt)
- **Precision:** Rounded to 0.2° for display
- **Range:** -15° to +15° (covers extreme cases up to 25% grade)

### Activity Tolerance Ranges

**Sleep:**
- Pitch: -2° to +2°
- Roll: -1° to +2°

**Cook:**
- Both axes: -1° to +1°

**Shower:**
- Pitch: -1° to 0°
- Roll: 0° to +1°

**Drain (Emptying/On Water):**
- Pitch: 0° or greater (>= 0°)
- Roll: 0° or less (<= 0°)

## MQTT Topics

### Published (Status & Sensor Data)

| Topic | Message Format | Update Frequency |
|-------|---------------|------------------|
| `smartcamper/sensors/module-3/status` | `{"type": "full", "data": {"circles": {...}}}` or `{"type": "circle", "index": 0, "mode": "TEMP_CONTROL", "relay": "ON", "temperature": 32, "error": false}` | On change (button press, MQTT command, automatic control change, force_update) |
| `smartcamper/sensors/module-3/leveling` | `{"pitch": 1.2, "roll": -0.8}` | Every 0.5 seconds when active (on-demand, timeout: 22 seconds) |
| `smartcamper/errors/module-3/circle/{index}` | `{"error": true, "type": "sensor_disconnected", "message": "Temperature sensor disconnected", "timestamp": 1234567890}` | Once when error occurs |
| `smartcamper/heartbeat/module-3` | `{"timestamp": 1234567890, "moduleId": "module-3", "uptime": 3600, "wifiRSSI": -65}` | Every 10 seconds |

### Subscribed (Commands)

| Topic | Payload | Action |
|-------|---------|--------|
| `smartcamper/commands/module-3/circle/{index}/on` | `{}` | Enable TEMP_CONTROL mode (temperature-based control) |
| `smartcamper/commands/module-3/circle/{index}/off` | `{}` | Disable circle (OFF mode) |
| `smartcamper/commands/module-3/leveling/start` | `{}` | Start leveling sensor (activates for 22 seconds, resets timeout) |
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
- **LevelingSensor**: MPU6050 sensor management, angle measurement, zeroing, on-demand data streaming

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

