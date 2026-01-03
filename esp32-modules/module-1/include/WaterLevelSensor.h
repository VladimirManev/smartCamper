// Water Level Sensor
// Specific sensor logic for gray water level detection using conductivity-based electrodes
// Handles: Reading, averaging, change detection, publishing

#ifndef WATER_LEVEL_SENSOR_H
#define WATER_LEVEL_SENSOR_H

#include "Config.h"
#include "MQTTManager.h"

class WaterLevelSensor {
private:
  MQTTManager* mqttManager;  // Reference to MQTT manager (not owned)
  
  // GPIO pins array (from bottom to top)
  int levelPins[NUM_LEVEL_PINS];
  int levelPercentages[NUM_LEVEL_PINS];
  
  // Timing
  unsigned long lastSensorRead;
  unsigned long lastDataSent;
  
  // Measurement data
  int levelIndices[5];  // Store last 5 level indices for averaging (not percentages)
  int measurementIndex;
  int measurementCount;
  
  // Last published value
  float lastPublishedLevel;
  bool forceUpdateRequested;
  bool lastMQTTState;  // Previous MQTT connection state (for detecting reconnects)
  
  // Sensor reading functions
  int readWaterLevel();  // Returns level index (0-6) or -1 if empty
  float levelToPercent(int level);  // Converts level index to percentage
  void setupPins();  // Configure pins as INPUT (no pull-up initially)
  void setPinsLow();  // Set all pins to LOW after measurement (corrosion prevention)
  
  // Statistics functions
  int findMode(int* values, int count);  // Find most frequent value in array
  
  // Publishing logic
  void publishIfNeeded(float modePercent, unsigned long currentTime, bool forcePublish = false);

public:
  WaterLevelSensor(MQTTManager* mqtt);
  
  // Initialization
  void begin();
  
  // Main loop - call this in your main loop()
  void loop();
  
  // Force update
  void forceUpdate();
  
  // Status (const methods)
  float getLastLevel() const { return lastPublishedLevel; }
  unsigned long getLastDataSent() const { return lastDataSent; }
  bool isForceUpdateRequested() const { return forceUpdateRequested; }
  void printStatus() const;
};

#endif

