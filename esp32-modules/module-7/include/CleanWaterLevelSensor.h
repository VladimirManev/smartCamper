// Clean Water Level Sensor
// Conductivity-based electrodes (same approach as module-1 gray water)

#ifndef CLEAN_WATER_LEVEL_SENSOR_H
#define CLEAN_WATER_LEVEL_SENSOR_H

#include "Config.h"
#include "MQTTManager.h"

class CleanWaterLevelSensor {
private:
  MQTTManager* mqttManager;

  int levelPins[NUM_CLEAN_WATER_LEVEL_PINS];
  int levelPercentages[NUM_CLEAN_WATER_LEVEL_PINS];

  unsigned long lastSensorRead;
  unsigned long lastDataSent;

  int levelIndices[CLEAN_WATER_LEVEL_MODE_SAMPLE_COUNT];
  int measurementIndex;
  int measurementCount;

  float lastPublishedLevel;
  bool forceUpdateRequested;
  bool lastMQTTState;

  int readWaterLevel();
  float levelToPercent(int level);
  void setupPins();
  void setPinsLow();
  int findMode(int* values, int count);
  void publishIfNeeded(float modePercent, unsigned long currentTime, bool forcePublish = false);

public:
  CleanWaterLevelSensor(MQTTManager* mqtt);

  void begin();
  void loop();
  void forceUpdate();

  float getLastLevel() const { return lastPublishedLevel; }
  unsigned long getLastDataSent() const { return lastDataSent; }
  void printStatus() const;
};

#endif
