# Heartbeat System Documentation

## Overview

The heartbeat system provides a standardized way for ESP32 modules to report their status to the backend. This ensures the backend can track which modules are online and detect when a module goes offline.

## Architecture

### Components

1. **HeartbeatManager** - Standalone class responsible for heartbeat functionality
2. **MQTT Communication** - Heartbeats are sent via MQTT on a dedicated topic
3. **Backend Monitoring** - Backend tracks last heartbeat timestamp for each module

### Design Principles

- **Separation of Concerns**: Heartbeat logic is isolated in its own class
- **Reusability**: Same implementation across all ESP32 modules
- **Independence**: Heartbeat works independently of sensor data publishing
- **Low Overhead**: Minimal CPU and network usage

## MQTT Topic Structure

### Heartbeat Topic

```
smartcamper/heartbeat/{module-id}
```

Examples:
- `smartcamper/heartbeat/module-1`
- `smartcamper/heartbeat/module-2`
- `smartcamper/heartbeat/module-3`

### Heartbeat Payload

JSON format:
```json
{
  "timestamp": 1234567890,
  "moduleId": "module-1",
  "uptime": 3600,
  "wifiRSSI": -65
}
```

Fields:
- `timestamp`: Unix timestamp (milliseconds since boot, or actual time if available)
- `moduleId`: Module identifier (e.g., "module-1")
- `uptime`: Module uptime in seconds
- `wifiRSSI`: WiFi signal strength in dBm (optional)

## Timing Configuration

| Setting | Value | Description |
|---------|-------|-------------|
| **Heartbeat Interval** | 10000 ms | Time between heartbeat messages (10 seconds) |
| **Backend Check Interval** | 5000 ms | How often backend checks for missing heartbeats |
| **Timeout Threshold** | 25000 ms | Time after which module is considered offline (2.5x heartbeat interval) |

## HeartbeatManager API

### Initialization

```cpp
HeartbeatManager heartbeatManager(&mqttManager, MODULE_ID);
heartbeatManager.begin();
```

### Main Loop

```cpp
void loop() {
  // Update heartbeat (call in main loop)
  heartbeatManager.loop();
}
```

### Methods

- `begin()` - Initialize heartbeat manager
- `loop()` - Update heartbeat (call in main loop)
- `setModuleId(String id)` - Set module identifier
- `forceSend()` - Force immediate heartbeat send
- `isEnabled()` - Check if heartbeat is enabled
- `getLastSentTime()` - Get timestamp of last heartbeat sent

## Integration Example

```cpp
#include "HeartbeatManager.h"
#include "Config.h"

class SensorManager {
private:
  MQTTManager mqttManager;
  HeartbeatManager heartbeatManager;
  
public:
  SensorManager() : heartbeatManager(&mqttManager, MODULE_ID) {
    // ...
  }
  
  void begin() {
    mqttManager.begin();
    heartbeatManager.begin();
    // ...
  }
  
  void loop() {
    mqttManager.loop();
    heartbeatManager.loop();  // Update heartbeat
    // ... sensor logic
  }
};
```

## Backend Behavior

1. **Heartbeat Reception**: Backend receives heartbeat on `smartcamper/heartbeat/{module-id}`
2. **Timestamp Tracking**: Backend stores last heartbeat timestamp for each module
3. **Periodic Check**: Backend checks every 5 seconds for missing heartbeats
4. **Offline Detection**: If no heartbeat received for 25 seconds, module is marked offline
5. **Status Broadcast**: Backend sends `moduleStatusUpdate` to frontend with all module statuses

## Error Handling

- **MQTT Not Connected**: Heartbeat is skipped (no error, will retry when connected)
- **WiFi Not Connected**: Heartbeat is skipped (no error, will retry when connected)
- **Publish Failure**: Silent failure, will retry on next interval

## Network Traffic Analysis

For 10 modules with 10-second heartbeat interval:
- Messages per minute: 60
- Bytes per message: ~150 (including MQTT overhead)
- Total per minute: ~9 KB
- Total per day: ~13 MB

**Conclusion**: Negligible network impact for local network usage.

## Future Enhancements

- Last Will and Testament (LWT) for unexpected disconnections
- Heartbeat with additional diagnostic information
- Adaptive heartbeat intervals based on network conditions
- Heartbeat acknowledgment from backend (optional)

