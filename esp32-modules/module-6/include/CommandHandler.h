// Command Handler
// Handle commands from Backend

#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include "Config.h"
#include "MQTTManager.h"

class VictronManager;

class CommandHandler {
 private:
  MQTTManager *mqttManager;
  VictronManager *victronManager;
  String moduleId;
  unsigned long lastForceUpdate;
  bool isSubscribed;

  static CommandHandler *currentInstance;

 public:
  CommandHandler(MQTTManager *mqtt, VictronManager *victronMgr, String moduleId);

  void begin();
  void loop();

  void handleMQTTMessage(char *topic, byte *payload, unsigned int length);
  static void handleMQTTMessageStatic(char *topic, byte *payload, unsigned int length);

  void forceUpdate();
  void printStatus() const;
};

#endif
