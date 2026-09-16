// Floor Heating Temperature Sensor
// DS18B20 per heating circle: burst of readings → median, periodic + on demand

#ifndef FLOOR_HEATING_SENSOR_H
#define FLOOR_HEATING_SENSOR_H

#include "Config.h"
#include "MQTTManager.h"
#include <OneWire.h>
#include <DallasTemperature.h>

class FloorHeatingController;
class FloorHeatingManager;

class FloorHeatingSensor {
private:
  MQTTManager* mqttManager;
  uint8_t circleIndex;
  uint8_t pin;
  OneWire oneWire;
  DallasTemperature sensors;

  float lastTemperature;             // Last accepted burst result (NAN = no data)
  float lastPublishedTemperature;    // Last value published to backend (NAN = none)
  bool forceUpdateRequested;
  bool immediateBurstRequested;      // Run a burst ASAP (ON, relay change, force)
  bool lastMQTTState;
  CircleMode lastKnownMode;

  // Burst state machine (non-blocking)
  bool burstInProgress;
  bool conversionStarted;
  unsigned long conversionStartTime;
  unsigned long lastBurstTime;       // When last burst finished (0 = never)
  uint8_t burstIndex;                // 0 .. HEATING_TEMP_BURST_COUNT-1
  float burstSamples[HEATING_TEMP_BURST_COUNT];

  int failedBurstCount;
  bool hasError;

  FloorHeatingController* controller;
  FloorHeatingManager* manager;

  static unsigned long globalRelaySettleUntil;
  static bool isGlobalRelaySettling();

  float readRawTemperature();
  bool isValidTemperature(float temp) const;
  float computeBurstResult() const;
  void clearMeasurementState();
  void abortBurst();
  void startBurst(unsigned long now);
  void startConversion(unsigned long now);
  void finishBurst(unsigned long now);
  void publishSensorError(const char* message);
  void publishIfNeeded(float temperature, bool forcePublish);

public:
  FloorHeatingSensor(MQTTManager* mqtt, uint8_t circleIndex, uint8_t pin);

  void begin();
  void loop();
  void forceUpdate();
  void onRelayChanged();
  static void beginGlobalRelaySettle();

  void setController(FloorHeatingController* ctrl);
  void setManager(FloorHeatingManager* mgr);

  float getLastTemperature() const { return lastTemperature; }
  float getLastPublishedTemperature() const { return lastPublishedTemperature; }
  bool hasSensorError() const { return hasError; }
  uint8_t getCircleIndex() const { return circleIndex; }
  void printStatus() const;

  void setLastPublishedTemperature(float temp) { lastPublishedTemperature = temp; }
};

#endif
