// Temperature Sensor Manager
// Специфична логика за температурен сензор

#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "../common/NetworkManager.h"
#include "../common/MQTTManager.h"

class SensorManager {
private:
  NetworkManager networkManager;
  MQTTManager mqttManager;
  
  unsigned long lastSensorRead;
  float lastTemperature;
  float lastHumidity;
  
  // Симулирани данни (за тест)
  float generateSimulatedTemperature();
  float generateSimulatedHumidity();

public:
  SensorManager();
  
  void begin();
  void loop();
  
  // MQTT callback за команди
  void handleMQTTMessage(char* topic, byte* payload, unsigned int length);
  
  void printStatus();
};

#endif
