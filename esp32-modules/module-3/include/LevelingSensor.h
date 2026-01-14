// Leveling Sensor
// GY-521 (MPU6050) sensor for camper leveling detection
// Measures pitch and roll angles for leveling indication

#ifndef LEVELING_SENSOR_H
#define LEVELING_SENSOR_H

#include "Config.h"
#include "MQTTManager.h"
#include <Wire.h>
#include <MPU6050_light.h>

class LevelingSensor {
private:
  MPU6050 mpu;
  MQTTManager* mqttManager;  // Reference to MQTT manager (not owned)
  unsigned long lastReadTime;
  unsigned long timeoutExpiresAt;  // When to stop publishing (0 = inactive)
  bool initialized;
  bool isActive;  // True if actively publishing
  
  // Sensor reading and publishing
  void readAndPublishSensor();

public:
  LevelingSensor(MQTTManager* mqtt);
  
  // Initialization
  void begin();
  
  // Main loop - call this in your main loop()
  void loop();
  
  // Start/activate leveling (resets timeout or starts if inactive)
  void start();
  
  // Status
  bool isInitialized() const { return initialized; }
  bool getIsActive() const { return isActive; }
  void printStatus() const;
};

#endif
