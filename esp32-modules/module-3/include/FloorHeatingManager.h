// Floor Heating Manager
// Coordinator for all floor heating functionality

#ifndef FLOOR_HEATING_MANAGER_H
#define FLOOR_HEATING_MANAGER_H

#include "Config.h"
#include "ModuleManager.h"
#include "FloorHeatingController.h"
#include "FloorHeatingSensor.h"
#include "FloorHeatingButtonHandler.h"
#include "CommandHandler.h"
#include "MQTTManager.h"
#include <ArduinoJson.h>

class FloorHeatingManager {
private:
  ModuleManager* moduleManager;
  FloorHeatingController controller;
  FloorHeatingSensor sensors[NUM_HEATING_CIRCLES];
  FloorHeatingButtonHandler buttonHandler;
  CommandHandler commandHandler;
  
  bool pendingStatusUpdate;  // Flag for deferred status publishing
  
  // Static pointer for MQTT callback
  static FloorHeatingManager* currentInstance;
  
  // MQTT message processing
  void processMQTTMessage(char* topic, byte* payload, unsigned int length);
  static void handleMQTTMessageStatic(char* topic, byte* payload, unsigned int length);
  
  // Command processing
  void handleCircleCommand(uint8_t circleIndex, String action, String payload);
  
public:
  FloorHeatingManager(ModuleManager* moduleMgr);
  
  // Initialization
  void begin();
  
  // Main loop - call this in your main loop()
  void loop();
  
  // Force update (for CommandHandler)
  void handleForceUpdate();
  
  // Get command handler reference (for ModuleManager)
  CommandHandler& getCommandHandler() { return commandHandler; }
  
  // Get controller reference (for status)
  FloorHeatingController& getController() { return controller; }
  
  // Status publishing (public for ButtonHandler and Controller callbacks)
  void publishFullStatus();
  void publishCircleStatus(uint8_t circleIndex);
  
  void printStatus() const;
};

#endif

