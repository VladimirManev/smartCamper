// Module Manager
// Common infrastructure manager for all ESP32 modules
// Handles: Network, MQTT, Heartbeat, Commands
// This class is shared across all modules

#ifndef MODULE_MANAGER_H
#define MODULE_MANAGER_H

#include "Config.h"
#include "NetworkManager.h"
#include "MQTTManager.h"
#include "HeartbeatManager.h"

// Forward declaration
class CommandHandler;

class ModuleManager {
private:
  NetworkManager networkManager;
  MQTTManager mqttManager;
  HeartbeatManager heartbeatManager;
  CommandHandler* commandHandler;  // Pointer - will be set by specific module logic
  
  bool initialized;
  bool lastConnectionState;  // Track previous connection state to detect reconnections

public:
  ModuleManager();
  
  // Initialization
  void begin();
  void begin(CommandHandler* cmdHandler);  // Overload with command handler
  
  // Main loop - call this in your main loop()
  void loop();
  
  // Getters for managers (for use by sensor classes)
  NetworkManager& getNetworkManager() { return networkManager; }
  MQTTManager& getMQTTManager() { return mqttManager; }
  HeartbeatManager& getHeartbeatManager() { return heartbeatManager; }
  
  // Status
  bool isConnected();  // Returns true if both WiFi and MQTT are connected (cannot be const - MQTTManager methods are not const)
  bool isInitialized() const { return initialized; }  // Check if module is initialized
  void printStatus();  // Cannot be const - MQTTManager methods are not const
};

#endif

