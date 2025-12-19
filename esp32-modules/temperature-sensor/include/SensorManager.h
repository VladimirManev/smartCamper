// Temperature Sensor Manager
// Specific logic for temperature sensor

#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "Config.h"
#include "NetworkManager.h"
#include "MQTTManager.h"
#include "CommandHandler.h"
#include "DHT.h"

class SensorManager {
private:
  NetworkManager networkManager;
  MQTTManager mqttManager;
  CommandHandler commandHandler;
  DHT dht;
  
  unsigned long lastSensorRead;
  unsigned long lastDataSent;        // Last data send
  unsigned long lastStatusLog;       // Last status log (when not connected)
  float lastTemperature;
  float lastHumidity;
  bool forceUpdateRequested;
  
  // Sensor functions
  float readTemperature();
  float readHumidity();

public:
  SensorManager();
  
  void begin();
  void loop();
  
  // Force update function (public for CommandHandler)
  void handleForceUpdate();
  
  // MQTT callback (static for MQTTManager)
  static void handleMQTTMessage(char* topic, byte* payload, unsigned int length);
  
  void printStatus();

private:
  // Static pointer to current instance
  static SensorManager* currentInstance;
};

#endif
