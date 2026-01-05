// Command Handler
// Handle commands from Backend

#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include "Config.h"
#include "MQTTManager.h"

// Forward declaration
class FloorHeatingManager;

class CommandHandler {
private:
  MQTTManager* mqttManager;
  FloorHeatingManager* floorHeatingManager;
  String moduleId;
  unsigned long lastForceUpdate;
  bool isSubscribed;  // Track if we're subscribed to command topics
  
  // Static pointer for MQTT callback
  static CommandHandler* currentInstance;

public:
  CommandHandler(MQTTManager* mqtt, FloorHeatingManager* floorHeating, String moduleId);
  
  void begin();
  void loop();
  
  // MQTT callback for commands
  void handleMQTTMessage(char* topic, byte* payload, unsigned int length);
  
  // Static callback for MQTTManager
  static void handleMQTTMessageStatic(char* topic, byte* payload, unsigned int length);
  
  // Force update function
  void forceUpdate();
  
  void printStatus() const;
};

#endif
