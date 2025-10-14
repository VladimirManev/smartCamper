// Temperature Sensor Manager
// Специфична логика за температурен сензор

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
  float lastTemperature;
  float lastHumidity;
  bool forceUpdateRequested;
  
  // Сензорни функции
  float readTemperature();
  float readHumidity();

public:
  SensorManager();
  
  void begin();
  void loop();
  
  // Force update функция (public за CommandHandler)
  void handleForceUpdate();
  
  // MQTT callback (static за MQTTManager)
  static void handleMQTTMessage(char* topic, byte* payload, unsigned int length);
  
  void printStatus();

private:
  // Статичен указател към текущия инстанс
  static SensorManager* currentInstance;
};

#endif
