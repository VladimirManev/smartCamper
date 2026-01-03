# Module 1 - ESP32 Sensor Module

## Overview

Module 1 is an ESP32-based sensor module designed for the SmartCamper system. It provides temperature and humidity monitoring using a DHT22/AM2301 sensor, with an extensible architecture that allows adding additional sensors.

## Architecture

### Core Components

1. **ModuleManager** - Common infrastructure (Network, MQTT, Heartbeat, Commands)
   - Handles WiFi connectivity
   - Manages MQTT communication
   - Sends heartbeat messages
   - Processes commands from backend

2. **SensorManager** - Sensor coordinator
   - Coordinates all sensor classes
   - Handles force update requests
   - Manages sensor lifecycle

3. **TemperatureHumiditySensor** - DHT22 sensor implementation
   - Reads temperature and humidity
   - Detects changes and publishes data
   - Handles sensor-specific logic

### Design Principles

- **Separation of Concerns**: Infrastructure (ModuleManager) is separate from sensor logic
- **Extensibility**: Easy to add new sensors without modifying existing code
- **Reusability**: ModuleManager can be copied to other modules as-is
- **Maintainability**: Clear structure and responsibilities

## Hardware Configuration

### DHT22/AM2301 Sensor
- **Pin**: GPIO 25 (configurable in `Config.h`)
- **Type**: DHT22 (configurable in `Config.h`)
- **Power**: 3.3V
- **Data**: Digital pin 25

## Software Configuration

### Module Identification
- **Module ID**: `module-1` (defined in `Config.h`)

### MQTT Topics

**Heartbeat:**
```
smartcamper/heartbeat/module-1
```

**Sensor Data:**
```
smartcamper/sensors/temperature
smartcamper/sensors/humidity
```

**Commands:**
```
smartcamper/commands/module-1/force_update
```

### Timing Configuration

- **Sensor Read Interval**: 1 second
- **Heartbeat Interval**: 10 seconds
- **Temperature Threshold**: 0.1°C (change detection)
- **Humidity Threshold**: 1% (change detection)

## Adding New Sensors

To add a new sensor (e.g., water level):

1. Create new sensor class:
```cpp
// include/WaterLevelSensor.h
class WaterLevelSensor {
  // Sensor-specific logic
};
```

2. Add to SensorManager:
```cpp
// include/SensorManager.h
class SensorManager {
  TemperatureHumiditySensor temperatureHumiditySensor;
  WaterLevelSensor waterLevelSensor;  // New sensor
};
```

3. Initialize and update in SensorManager:
```cpp
void SensorManager::loop() {
  temperatureHumiditySensor.loop();
  waterLevelSensor.loop();  // New sensor
}
```

## Building and Uploading

```bash
# Build project
pio run

# Upload to ESP32
pio run -t upload

# Monitor serial output
pio device monitor
```

## Dependencies

- `knolleary/PubSubClient@^2.8` - MQTT client
- `adafruit/DHT sensor library@^1.4.4` - DHT sensor library
- `adafruit/Adafruit Unified Sensor@^1.1.14` - Unified sensor library
- `bblanchon/ArduinoJson@^6.21.3` - JSON parsing

## File Structure

```
module-1/
├── include/
│   ├── CommandHandler.h          # Command processing
│   ├── HeartbeatManager.h        # Heartbeat functionality
│   ├── ModuleManager.h           # Infrastructure manager
│   ├── SensorManager.h           # Sensor coordinator
│   └── TemperatureHumiditySensor.h # DHT sensor logic
├── src/
│   ├── CommandHandler.cpp
│   ├── Config.h                  # Configuration constants
│   ├── HeartbeatManager.cpp
│   ├── main.cpp                  # Entry point
│   ├── ModuleManager.cpp
│   ├── MQTTManager.cpp/h         # MQTT communication
│   ├── NetworkManager.cpp/h       # WiFi management
│   ├── SensorManager.cpp
│   └── TemperatureHumiditySensor.cpp
├── platformio.ini                # PlatformIO configuration
├── README.md                     # This file (English)
├── README_BG.md                  # This file (Bulgarian)
├── HEARTBEAT_DOCUMENTATION.md    # Heartbeat system docs (English)
└── HEARTBEAT_DOCUMENTATION_BG.md # Heartbeat system docs (Bulgarian)
```

## Code Quality & Safety Features

The module includes the following production-ready improvements:

- **Null Pointer Validation**: Comprehensive nullptr checks in all critical paths
- **Input Validation**: Parameter validation in constructors and sensor value bounds checking
- **Error Handling**: Improved error handling with clear error messages
- **Initialization Checks**: State validation to prevent usage before initialization
- **Const Correctness**: Proper use of const methods where appropriate
- **Bounds Checking**: Sensor value validation (temperature: -40 to 80°C, humidity: 0-100%)

## Notes

- Module ID is defined in `Config.h` as `MODULE_ID`
- DHT sensor pin and type are configurable in `Config.h`
- All sensor-specific thresholds are in `Config.h`
- ModuleManager is reusable across all modules
- Sensor classes are independent and can be added/removed easily
- All documentation is available in both English and Bulgarian

