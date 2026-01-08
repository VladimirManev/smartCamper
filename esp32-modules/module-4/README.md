# Module 4 - Base Module

ESP32 module with base infrastructure ready for extension.

## Overview

Module 4 is a base module that includes all common infrastructure components:
- Network management (WiFi)
- MQTT communication
- Heartbeat system
- Command handling

This module can be extended with specific sensors/actuators as needed.

## Architecture

- **ModuleManager**: Handles common infrastructure (Network, MQTT, Heartbeat, Commands)
- **SensorManager**: Base coordinator (can be extended with specific functionality)
- **CommandHandler**: Handles MQTT commands from backend
- **NetworkManager**: WiFi connection management
- **MQTTManager**: MQTT broker communication
- **HeartbeatManager**: Module status reporting

## Configuration

Edit `src/Config.h` to configure:
- WiFi credentials
- MQTT broker IP and port
- Module-specific settings (when added)

## Building and Uploading

1. Install PlatformIO
2. Open the project in PlatformIO
3. Build: `pkgio run`
4. Upload: `pkgio run --target upload`
5. Monitor: `pkgio device monitor`

## MQTT Topics

- Heartbeat: `smartcamper/heartbeat/module-4`
- Commands: `smartcamper/commands/module-4/#`
- Sensors: `smartcamper/sensors/{sensor-type}` (when sensors are added)

## Extension

To add specific functionality:

1. Add sensor/actuator classes in `include/` and `src/`
2. Update `SensorManager` to include new components
3. Add configuration in `Config.h`
4. Update `main.cpp` if needed

## Status

✅ Base infrastructure complete
✅ WiFi connection management
✅ MQTT communication
✅ Heartbeat system
✅ Command handling
⏳ Ready for specific functionality implementation

