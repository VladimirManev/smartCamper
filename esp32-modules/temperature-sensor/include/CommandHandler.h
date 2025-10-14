// Command Handler
// Обработка на команди от Backend

#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include "Config.h"
#include "MQTTManager.h"

// Forward declaration
class SensorManager;

class CommandHandler {
private:
  MQTTManager* mqttManager;
  SensorManager* sensorManager;
  String moduleType;
  unsigned long lastForceUpdate;

public:
  CommandHandler(MQTTManager* mqtt, SensorManager* sensor, String moduleType);
  
  void begin();
  void loop();
  
  // MQTT callback за команди
  void handleMQTTMessage(char* topic, byte* payload, unsigned int length);
  
  // Force update функция
  void forceUpdate();
  
  void printStatus();
};

#endif
