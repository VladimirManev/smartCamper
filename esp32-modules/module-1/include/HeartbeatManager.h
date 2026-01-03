// Heartbeat Manager
// Standalone class for managing module heartbeat functionality
// Provides standardized heartbeat mechanism for all ESP32 modules

#ifndef HEARTBEAT_MANAGER_H
#define HEARTBEAT_MANAGER_H

#include <Arduino.h>
#include "MQTTManager.h"
#include "Config.h"

class HeartbeatManager {
private:
  MQTTManager* mqttManager;  // Reference to MQTT manager (not owned)
  String moduleId;            // Module identifier (e.g., "module-1")
  
  unsigned long lastHeartbeatSent;  // Timestamp of last heartbeat sent
  bool enabled;                     // Whether heartbeat is enabled
  unsigned long uptimeStart;        // Module start time for uptime calculation
  bool lastMQTTState;               // Previous MQTT connection state (for detecting reconnects)

  // Internal methods
  bool shouldSendHeartbeat();
  void sendHeartbeat();
  String buildHeartbeatPayload();
  unsigned long getUptimeSeconds() const;

public:
  // Constructor
  HeartbeatManager(MQTTManager* mqtt, String moduleId);
  
  // Initialization
  void begin();
  
  // Main loop - call this in your main loop()
  void loop();
  
  // Configuration
  void setModuleId(String id);
  void setEnabled(bool enabled);
  
  // Control
  void forceSend();  // Force immediate heartbeat send
  
  // Status
  bool isEnabled() const;
  unsigned long getLastSentTime() const;
  String getModuleId() const;
  
  // Debug
  void printStatus() const;
};

#endif

