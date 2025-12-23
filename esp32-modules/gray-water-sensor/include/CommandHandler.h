// Command Handler
// Handle commands from Backend

#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include "Config.h"
#include "MQTTManager.h"

// Forward declaration
class WaterLevelManager;

class CommandHandler {
private:
  MQTTManager* mqttManager;
  WaterLevelManager* waterLevelManager;
  String moduleType;
  unsigned long lastForceUpdate;

public:
  CommandHandler(MQTTManager* mqtt, WaterLevelManager* waterLevel, String moduleType);
  
  void begin();
  void loop();
  
  // MQTT callback for commands
  void handleMQTTMessage(char* topic, byte* payload, unsigned int length);
  
  // Force update function
  void forceUpdate();
  
  void printStatus();
};

#endif
