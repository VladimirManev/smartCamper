// Appliance Manager
// Coordinator for all appliance-related functionality
// Manages RelayController, ButtonHandler, and CommandHandler

#ifndef APPLIANCE_MANAGER_H
#define APPLIANCE_MANAGER_H

#include "Config.h"
#include "ModuleManager.h"
#include "RelayController.h"
#include "ButtonHandler.h"
#include "CommandHandler.h"

class ApplianceManager {
private:
  ModuleManager* moduleManager;  // Reference to module manager (not owned)
  RelayController relayController;
  ButtonHandler buttonHandler;
  CommandHandler commandHandler;
  
  // Static pointer for MQTT callback
  static ApplianceManager* currentInstance;
  
  // Flag to defer status publishing (to avoid publishing during MQTT callback)
  bool pendingStatusUpdate;

public:
  ApplianceManager(ModuleManager* moduleMgr);
  
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
  
  // Process appliance-specific MQTT commands
  void processApplianceCommand(char* topic, byte* payload, unsigned int length);
  
  // Getter for CommandHandler (needed by ModuleManager)
  CommandHandler& getCommandHandler() { return commandHandler; }
  
  // Getters for components
  RelayController& getRelayController() { return relayController; }
  ButtonHandler& getButtonHandler() { return buttonHandler; }
  
  // Publish status (for CommandHandler and status updates)
  void publishFullStatus();
  void publishRelayStatus();
  
  // Check if any button is pressed (for MQTT command blocking)
  bool isAnyButtonPressed() const;
  
  // Status
  void printStatus() const;
};

#endif
