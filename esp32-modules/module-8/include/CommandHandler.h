#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include "Config.h"
#include "MQTTManager.h"

class AlarmManager;

class CommandHandler {
private:
  MQTTManager* mqttManager;
  AlarmManager* alarmManager;
  String moduleId;
  unsigned long lastForceUpdate;
  bool isSubscribed;

  static CommandHandler* currentInstance;

  void handleZone1On(const String& message);
  void handleZone1Off(const String& message);
  void handleZone2On();
  void handleZone2Off();

public:
  CommandHandler(MQTTManager* mqtt, AlarmManager* manager, String moduleId);

  void begin();
  void loop();

  void handleMQTTMessage(char* topic, byte* payload, unsigned int length);
  static void handleMQTTMessageStatic(char* topic, byte* payload, unsigned int length);

  void forceUpdate();
  void printStatus() const;
};

#endif
