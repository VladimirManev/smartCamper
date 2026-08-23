// Core alarm state machine (zone 1 + zone 2 + cat mode)

#ifndef ALARM_SYSTEM_H
#define ALARM_SYSTEM_H

#include <Arduino.h>
#include "Config.h"
#include "BuzzerController.h"
#include "AlarmButtonHandler.h"

enum Zone1Phase {
  Z1_IDLE = 0,
  Z1_EXIT_DELAY,
  Z1_ARMED,
  Z1_ENTRY_DELAY,
  Z1_ALARM
};

class AlarmSystem {
private:
  BuzzerController* buzzer;
  AlarmButtonHandler button;

  Zone1Phase zone1Phase;
  bool zone1IgnoreInteriorPir;
  bool zone2Armed;

  bool sirenOn;
  bool smokeOn;
  bool zone1LedOn;
  unsigned long zone1LedLastToggle;
  unsigned long phaseStartMs;
  unsigned long sirenStartMs;
  unsigned long smokeStartMs;
  unsigned long lastPerimeterAlertMs;

  // Inputs
  bool spareOpen;
  bool lastSpareOpen;
  bool spareLatchedOpen;  // ignore until closed after alarm cycle
  bool interiorPir;
  bool perimeterPir[NUM_PERIMETER_PIRS];
  bool lastPerimeterPir[NUM_PERIMETER_PIRS];

  bool statusDirty;
  bool waitingConfirmThenExit;  // after confirm beep, start exit delay
  bool pendingArmIsCat;

  static const char* perimeterNames[NUM_PERIMETER_PIRS];
  static const uint8_t perimeterPins[NUM_PERIMETER_PIRS];

  void readInputs();
  void updateZone1Led();
  void setSiren(bool on);
  void setSmoke(bool on);
  void startExitDelay();
  void startEntryDelay();
  void startAlarm();
  void stopAlarmOutputs();
  void disarmZone1();
  void handleButtonAction(AlarmButtonAction action);
  void processZone1Sensors();
  void processPerimeter();
  void updateExitEntryDelay();
  void updateAlarmPhase();
  const char* phaseToString() const;

public:
  AlarmSystem(BuzzerController* buzzerCtrl);

  void begin();
  void loop();

  // MQTT / external control — returns false on reject (error beep)
  bool armZone1(bool ignoreInteriorPir);
  bool disarmZone1WithPin(const char* pin);
  bool armZone2();
  bool disarmZone2();

  void markStatusDirty() { statusDirty = true; }
  bool consumeStatusDirty();

  // Status getters for MQTT
  Zone1Phase getZone1Phase() const { return zone1Phase; }
  bool isZone1Armed() const { return zone1Phase != Z1_IDLE; }
  bool getIgnoreInteriorPir() const { return zone1IgnoreInteriorPir; }
  bool isZone2Armed() const { return zone2Armed; }
  bool isSirenOn() const { return sirenOn; }
  bool isSmokeOn() const { return smokeOn; }
  bool isSpareOpen() const { return spareOpen; }
  bool isInteriorPir() const { return interiorPir; }
  bool getPerimeterPir(uint8_t i) const;
  const char* getPhaseString() const { return phaseToString(); }
  bool isArmPending() const { return waitingConfirmThenExit; }
};

#endif
