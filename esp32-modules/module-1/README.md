# Module-1: Environmental Sensors

ESP32 module for monitoring indoor temperature, indoor humidity, outdoor temperature, water level, and water temperature.

## Hardware Configuration

### Pin Assignment

| Component | Pin | Type | Description |
|-----------|-----|------|-------------|
| **DHT22/AM2301** | 25 | Input | Indoor temperature & Humidity sensor |
| **DS18B20 (Water)** | 26 | Input | Gray water temperature sensor (OneWire) |
| **DS18B20 (Outdoor)** | 27 | Input | Outdoor temperature sensor (OneWire) |
| **Water Level 1** | 4 | Input | 15% level detection |
| **Water Level 2** | 5 | Input | 30% level detection |
| **Water Level 3** | 18 | Input | 45% level detection |
| **Water Level 4** | 19 | Input | 60% level detection |
| **Water Level 5** | 21 | Input | 75% level detection |
| **Water Level 6** | 22 | Input | 90% level detection |
| **Water Level 7** | 23 | Input | 100% level detection |

### Sensor Specifications

**DHT22/AM2301 (Indoor):**
- Temperature range: -40°C to 80°C
- Humidity range: 0-100% RH
- Accuracy: ±0.5°C, ±1% RH
- Update interval: 1 second

**DS18B20 (Gray Water):**
- Temperature range: -55°C to 125°C
- Accuracy: ±0.5°C
- Update interval: 1 second (5-second average)

**DS18B20 (Outdoor):**
- Temperature range: -55°C to 125°C
- Accuracy: ±0.5°C
- Update interval: 1 second (5-second average)

**Water Level Sensor:**
- 7-level detection (15%, 30%, 45%, 60%, 75%, 90%, 100%)
- Digital pins with pull-up resistors
- Update interval: 1 second (5-second average)

## Network Configuration

- **WiFi SSID**: `SmartCamper`
- **WiFi Password**: `12344321`
- **MQTT Broker IP**: `192.168.4.1` (Raspberry Pi)
- **MQTT Broker Port**: `1883`
- **Module ID**: `module-1`

## MQTT Topics

### Published (Sensor Data)

| Topic | Message Format | Update Frequency |
|-------|---------------|------------------|
| `smartcamper/sensors/module-1/indoor-temperature` | `{"value": 22.5, "timestamp": 1234567890}` | On change (>0.1°C) |
| `smartcamper/sensors/module-1/indoor-humidity` | `{"value": 45.0, "timestamp": 1234567890}` | On change (>1%) |
| `smartcamper/sensors/module-1/outdoor-temperature` | `{"value": 18.5, "timestamp": 1234567890}` | On change (>0.1°C) |
| `smartcamper/sensors/module-1/gray-water/level` | `{"value": 75, "timestamp": 1234567890}` | On change (>1%) |
| `smartcamper/sensors/module-1/gray-water-temperature` | `{"value": 18.5, "timestamp": 1234567890}` | On change (>0.1°C) |
| `smartcamper/heartbeat/module-1` | `{"timestamp": 1234567890, "moduleId": "module-1", "uptime": 3600, "wifiRSSI": -65}` | Every 10 seconds |

### Subscribed (Commands)

| Topic | Payload | Action |
|-------|---------|--------|
| `smartcamper/commands/module-1/force_update` | `{}` | Force publish all sensor data |

## Installation & Setup

1. **Upload firmware:**
   ```bash
   cd esp32-modules/module-1
   pio run --target upload
   ```

2. **Monitor serial output:**
   ```bash
   pio device monitor
   ```
   - Check WiFi connection status
   - Verify MQTT connection
   - Monitor sensor readings

3. **Verify operation:**
   - Check frontend dashboard for sensor data
   - Verify heartbeat status icon (blue = online, red = offline)

## Troubleshooting

### No WiFi Connection
- Check SSID and password in `src/Config.h`
- Verify WiFi router is broadcasting `SmartCamper` network
- Check serial output for connection errors

### No MQTT Connection
- Verify MQTT broker IP address in `src/Config.h` (default: `192.168.4.1`)
- Check if Raspberry Pi is running and MQTT broker is active
- Verify network connectivity (ping MQTT broker IP)

### No Sensor Readings
- **DHT22 (Indoor)**: Check wiring (VCC, GND, DATA pin 25)
- **DS18B20 (Water)**: Verify OneWire pull-up resistor (4.7kΩ) and pin 26 connection
- **DS18B20 (Outdoor)**: Verify OneWire pull-up resistor (4.7kΩ) and pin 27 connection
- **Water Level**: Check all 7 pins (4, 5, 18, 19, 21, 22, 23) are connected
- Check serial output for sensor errors

### Sensor Shows Wrong Values
- **Indoor Temperature/Humidity**: DHT22 may need 2-3 seconds between readings
- **Water Level**: Check sensor connections and water contact
- **Water Temperature**: Verify DS18B20 (pin 26) is properly sealed and immersed
- **Outdoor Temperature**: Verify DS18B20 (pin 27) is properly positioned and connected

## Offline Operation

Module operates independently:
- Sensors continue reading even without WiFi/MQTT
- Data is buffered and published when connection is restored
- Auto-reconnect to WiFi and MQTT every 2-3 seconds

## Architecture

- **ModuleManager**: Handles WiFi, MQTT, Heartbeat, Commands
- **SensorManager**: Coordinates all sensors (DHT22, Water Level, DS18B20 Water, DS18B20 Outdoor)
- **NetworkManager**: WiFi connection and reconnection
- **MQTTManager**: MQTT communication and auto-reconnect
- **HeartbeatManager**: Sends status every 10 seconds

## Serial Debug Output

Enable/disable in `src/Config.h`:
- `DEBUG_SERIAL`: Serial console output
- `DEBUG_MQTT`: MQTT publish/subscribe messages
- `DEBUG_VERBOSE`: Detailed sensor readings

