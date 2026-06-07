// Urine Level Sensor
// Toilet urine tank level via conductivity electrodes (same approach as module-1 gray water)

#ifndef URINE_LEVEL_SENSOR_H
#define URINE_LEVEL_SENSOR_H

#include "Config.h"
#include "MQTTManager.h"

class UrineLevelSensor {
private:
  MQTTManager* mqttManager;

  int levelPins[NUM_URINE_LEVEL_PINS];
  int levelPercentages[NUM_URINE_LEVEL_PINS];

  unsigned long lastSensorRead;
  unsigned long lastDataSent;

  int levelIndices[URINE_LEVEL_MODE_SAMPLE_COUNT];
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
  UrineLevelSensor(MQTTManager* mqtt);

  void begin();
  void loop();
  void forceUpdate();

  float getLastLevel() const { return lastPublishedLevel; }
  void printStatus() const;
};

#endif
