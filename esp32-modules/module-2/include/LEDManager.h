// LED Manager
// Coordinator for all LED-related functionality
// Manages LEDStripController, ButtonHandler, RelayController, PIRSensorHandler, and CommandHandler

#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include "Config.h"
#include "ModuleManager.h"
#include "LEDStripController.h"
#include "ButtonHandler.h"
#include "RelayController.h"
#include "PIRSensorHandler.h"
#include "CommandHandler.h"
#include "StripState.h"

class LEDManager {
private:
  ModuleManager* moduleManager;  // Reference to module manager (not owned)
  LEDStripController ledStripController;
  RelayController relayController;
  ButtonHandler buttonHandler;
  PIRSensorHandler pirSensorHandler;
  CommandHandler commandHandler;
  
  // Static pointer for MQTT callback
  static LEDManager* currentInstance;

public:
  LEDManager(ModuleManager* moduleMgr);
  
  // Initialization
  void begin();
  
  // Main loop - call this in your main loop()
  void loop();
  
  // Force update (for CommandHandler)
  void handleForceUpdate();
  
  // MQTT callback (static for MQTTManager)
  static void handleMQTTMessageStatic(char* topic, byte* payload, unsigned int length);
  
  // Process MQTT messages (called from static callback)
  void processMQTTMessage(char* topic, byte* payload, unsigned int length);
  
  // Process LED-specific MQTT commands
  void processLEDCommand(char* topic, byte* payload, unsigned int length);
  
  // Getter for CommandHandler (needed by ModuleManager)
  CommandHandler& getCommandHandler() { return commandHandler; }
  
  // Getters for components
  LEDStripController& getLEDStripController() { return ledStripController; }
  RelayController& getRelayController() { return relayController; }
  ButtonHandler& getButtonHandler() { return buttonHandler; }
  PIRSensorHandler& getPIRSensorHandler() { return pirSensorHandler; }
  
  // Publish status (for CommandHandler and status updates)
  void publishFullStatus();
  void publishStripStatus(uint8_t stripIndex);
  void publishRelayStatus();
  
  // Check if any button is pressed (for MQTT command blocking)
  bool isAnyButtonPressed() const;
  
  // Status
  void printStatus() const;
};

#endif

