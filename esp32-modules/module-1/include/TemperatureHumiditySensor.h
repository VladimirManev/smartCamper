// Temperature and Humidity Sensor
// Specific sensor logic for DHT22/AM2301 sensor
// Handles: Reading, change detection, publishing

#ifndef TEMPERATURE_HUMIDITY_SENSOR_H
#define TEMPERATURE_HUMIDITY_SENSOR_H

#include "Config.h"
#include "MQTTManager.h"
#include "DHT.h"

class TemperatureHumiditySensor {
private:
  MQTTManager* mqttManager;  // Reference to MQTT manager (not owned)
  DHT dht;
  
  unsigned long lastSensorRead;
  unsigned long lastDataSent;
  float lastTemperature;
  float lastHumidity;
  bool forceUpdateRequested;
  
  // Sensor reading functions
  float readTemperature();
  float readHumidity();
  
  // Publishing logic
  void publishIfNeeded(float temperature, float humidity, unsigned long currentTime, bool forcePublish = false);

public:
  TemperatureHumiditySensor(MQTTManager* mqtt, uint8_t pin, uint8_t type);
  
  // Initialization
  void begin();
  
  // Main loop - call this in your main loop()
  void loop();
  
  // Force update
  void forceUpdate();
  
  // Status (const methods)
  float getLastTemperature() const { return lastTemperature; }
  float getLastHumidity() const { return lastHumidity; }
  unsigned long getLastDataSent() const { return lastDataSent; }
  bool isForceUpdateRequested() const { return forceUpdateRequested; }
  void printStatus() const;
};

#endif

