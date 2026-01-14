// Leveling Sensor
// GY-521 (MPU6050) sensor for camper leveling detection
// Measures pitch and roll angles for leveling indication

#ifndef LEVELING_SENSOR_H
#define LEVELING_SENSOR_H

#include "Config.h"
#include "MQTTManager.h"
#include <Wire.h>
#include <MPU6050_light.h>
#include <Preferences.h>

class LevelingSensor {
private:
  MPU6050 mpu;
  MQTTManager* mqttManager;  // Reference to MQTT manager (not owned)
  Preferences preferences;    // For storing zero offsets in flash
  unsigned long lastReadTime;
  unsigned long timeoutExpiresAt;  // When to stop publishing (0 = inactive)
  bool initialized;
  bool isActive;  // True if actively publishing
  
  // Zero offset values (stored in flash)
  float pitchOffset;
  float rollOffset;
  
  // Button handling
  bool buttonPressed;
  unsigned long buttonPressStartTime;
  bool zeroingInProgress;
  
  // Sensor reading and publishing
  void readAndPublishSensor();
  
  // Zero offset management
  void loadZeroOffsets();
  void saveZeroOffsets(float pitch, float roll);
  void handleZeroButton();
  void blinkLED(int times, int duration);

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
  float getPitchOffset() const { return pitchOffset; }
  float getRollOffset() const { return rollOffset; }
  void printStatus() const;
};

#endif
