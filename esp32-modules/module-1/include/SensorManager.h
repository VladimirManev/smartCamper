// Sensor Manager
// Coordinator for all sensor classes
// Manages multiple sensors and handles commands

#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "Config.h"
#include "ModuleManager.h"
#include "TemperatureHumiditySensor.h"
#include "CommandHandler.h"

class SensorManager {
private:
  ModuleManager* moduleManager;  // Reference to module manager (not owned)
  TemperatureHumiditySensor temperatureHumiditySensor;
  CommandHandler commandHandler;
  
  // Static pointer for MQTT callback
  static SensorManager* currentInstance;

public:
  SensorManager(ModuleManager* moduleMgr);
  
  // Initialization
  void begin();
  
  // Main loop - call this in your main loop()
  void loop();
  
  // Force update (for CommandHandler)
  void handleForceUpdate();
  
  // MQTT callback (static for MQTTManager)
  static void handleMQTTMessage(char* topic, byte* payload, unsigned int length);
  
  // Getter for CommandHandler (needed by ModuleManager)
  CommandHandler& getCommandHandler() { return commandHandler; }
  
  // Status
  void printStatus() const;
};

#endif
