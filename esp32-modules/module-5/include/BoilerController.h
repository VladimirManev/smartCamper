// Boiler thermostat. Owns the element relay (GPIO 27), two DS18B20 buses,
// and the full-tank electrode. Relay 4 is on while a mode is active
// (maintain or one-shot). The element is a separate flag.

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
  void requestOnce(int target);
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

  enum Mode {
    MODE_OFF,
    MODE_MAINTAIN,
    MODE_ONCE
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
  Mode mode;
  int onceTarget;
  bool elementOn;
  bool enabledChanged;
  int setpoint;
  bool hasFull;
  bool full;
  Error error;

  bool burstActive;
  bool waitingConversion;
  bool immediateBurst;
  bool bottomConverting;
  bool topConverting;
  bool bottomSampleDone;
  bool topSampleDone;
  uint8_t bottomTries;
  uint8_t topTries;
  uint8_t burstIndex;
  unsigned long conversionStartedMs;
  unsigned long lastBurstMs;
  String lastPayload;

  void startBurst(unsigned long now);
  void beginConversion(unsigned long now);
  void collectSample();
  void finishBurst(unsigned long now);
  void applyControl();
  void trip(Error code);
  void setMode(Mode next);
  void setElement(bool on);
  void closeElementIfFull();
  const char* modeName() const;
  void publish(bool force);

  bool readElectrodeFull();
  static float readCelsius(DallasTemperature& sensors);
  static bool median(const float* samples, int* roundedOut);
  static void clearSamples(TempChannel& channel);
  const char* errorName() const;
};

#endif
