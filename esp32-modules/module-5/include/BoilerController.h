// Boiler thermostat. Owns the element relay (GPIO 27), two DS18B20 buses,
// and the full-tank electrode. The published relay-4 state is "enabled"
// (thermostat armed). The element is a separate flag.

#ifndef BOILER_CONTROLLER_H
#define BOILER_CONTROLLER_H

#include "Config.h"
#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>

class MQTTManager;
class ApplianceManager;

class BoilerController {
public:
  BoilerController(MQTTManager* mqtt, ApplianceManager* appMgr);

  void begin();
  void loop();

  void requestOn();
  void requestOff();
  void onButton();
  void setSetpoint(int value);
  void onForceUpdate();

  bool isEnabled() const { return enabled; }

private:
  enum Error {
    ERROR_NONE,
    ERROR_EMPTY,
    ERROR_SENSOR,
    ERROR_OVERTEMP
  };

  struct TempChannel {
    int reading;
    bool hasReading;
    uint8_t failCount;
    float samples[BOILER_TEMP_BURST_COUNT];
  };

  MQTTManager* mqttManager;
  ApplianceManager* applianceManager;

  OneWire bottomWire;
  OneWire topWire;
  DallasTemperature bottomSensors;
  DallasTemperature topSensors;

  TempChannel bottom;
  TempChannel top;

  bool enabled;
  bool elementOn;
  bool enabledChanged;
  int setpoint;
  bool hasFull;
  bool full;
  Error error;

  bool burstActive;
  bool waitingConversion;
  bool immediateBurst;
  uint8_t burstIndex;
  unsigned long conversionStartedMs;
  unsigned long lastBurstMs;
  String lastPayload;

  void startBurst(unsigned long now);
  void finishBurst(unsigned long now);
  void applyControl();
  void trip(Error code);
  void setEnabled(bool on);
  void setElement(bool on);
  void publish(bool force);

  bool readElectrodeFull();
  static float readCelsius(DallasTemperature& sensors);
  static bool median(const float* samples, int* roundedOut);
  static void clearSamples(TempChannel& channel);
  const char* errorName() const;
};

#endif
