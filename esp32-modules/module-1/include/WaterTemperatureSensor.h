// Water Temperature Sensor
// DS18B20: burst of readings → median, every DS18B20_TEMP_INTERVAL_MS

#ifndef WATER_TEMPERATURE_SENSOR_H
#define WATER_TEMPERATURE_SENSOR_H

#include "Config.h"
#include "MQTTManager.h"
#include <OneWire.h>
#include <DallasTemperature.h>

class WaterTemperatureSensor {
private:
  MQTTManager* mqttManager;
  OneWire oneWire;
  DallasTemperature sensors;

  float lastTemperature;           // Last accepted burst result (NAN = none)
  float lastPublishedTemperature;  // Last value sent to MQTT (NAN = none)
  unsigned long lastDataSent;
  bool forceUpdateRequested;
  bool immediateBurstRequested;
  bool lastMQTTState;

  bool burstInProgress;
  bool conversionStarted;
  unsigned long conversionStartTime;
  unsigned long lastBurstTime;
  uint8_t burstIndex;
  float burstSamples[DS18B20_TEMP_BURST_COUNT];
  int failedBurstCount;

  float readRawTemperature();
  bool isValidTemperature(float temp) const;
  float computeBurstResult() const;
  void abortBurst();
  void startBurst(unsigned long now);
  void startConversion(unsigned long now);
  void finishBurst(unsigned long now);
  void publishIfNeeded(float temperature, unsigned long now, bool forcePublish);

public:
  WaterTemperatureSensor(MQTTManager* mqtt);

  void begin();
  void loop();
  void forceUpdate();

  float getLastTemperature() const { return lastTemperature; }
  unsigned long getLastDataSent() const { return lastDataSent; }
  bool isForceUpdateRequested() const { return forceUpdateRequested; }
  void printStatus() const;
};

#endif
