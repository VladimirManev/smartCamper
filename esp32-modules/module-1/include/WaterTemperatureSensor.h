// Water Temperature Sensor
// Specific sensor logic for DS18B20 sensor (OneWire)
// Handles: Reading, averaging, change detection, publishing

#ifndef WATER_TEMPERATURE_SENSOR_H
#define WATER_TEMPERATURE_SENSOR_H

#include "Config.h"
#include "MQTTManager.h"
#include <OneWire.h>
#include <DallasTemperature.h>

class WaterTemperatureSensor {
private:
  MQTTManager* mqttManager;  // Reference to MQTT manager (not owned)
  OneWire oneWire;
  DallasTemperature sensors;
  
  unsigned long lastSensorRead;
  unsigned long lastDataSent;
  float lastTemperature;
  bool forceUpdateRequested;
  bool lastMQTTState;  // Previous MQTT connection state (for detecting reconnects)
  
  // Async temperature reading state machine
  bool conversionStarted;  // True if we've started a temperature conversion
  unsigned long conversionStartTime;  // When we started the conversion
  
  // Temperature averaging
  float temperatureReadings[WATER_TEMP_AVERAGE_COUNT];
  int temperatureIndex;
  int temperatureCount;
  unsigned long lastAverageTime;
  
  // Sensor reading functions
  float readTemperature();
  
  // Averaging functions
  float calculateAverageTemperature();
  
  // Publishing logic
  void publishIfNeeded(float temperature, unsigned long currentTime, bool forcePublish = false);

public:
  WaterTemperatureSensor(MQTTManager* mqtt);
  
  // Initialization
  void begin();
  
  // Main loop - call this in your main loop()
  void loop();
  
  // Force update
  void forceUpdate();
  
  // Status (const methods)
  float getLastTemperature() const { return lastTemperature; }
  unsigned long getLastDataSent() const { return lastDataSent; }
  bool isForceUpdateRequested() const { return forceUpdateRequested; }
  void printStatus() const;
};

#endif

