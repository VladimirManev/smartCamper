// Non-blocking buzzer patterns for alarm module

#ifndef BUZZER_CONTROLLER_H
#define BUZZER_CONTROLLER_H

#include <Arduino.h>

enum BuzzerPattern {
  BUZZ_IDLE = 0,
  BUZZ_ERROR,
  BUZZ_CONFIRM_Z1_ON,
  BUZZ_CONFIRM_Z1_OFF,
  BUZZ_CONFIRM_Z2_ON,
  BUZZ_CONFIRM_Z2_OFF,
  BUZZ_CONFIRM_CAT_ON,
  BUZZ_CONFIRM_CAT_OFF,
  BUZZ_DELAY_ESCALATION,  // 10s 1/s, 10s 2/s, 10s continuous
  BUZZ_PERIMETER_ALERT    // 5 short beeps
};

class BuzzerController {
private:
  BuzzerPattern pattern;
  unsigned long patternStart;
  unsigned long nextToggleAt;
  bool pinHigh;
  bool busy;
  uint8_t step;
  uint8_t shortBeepRemaining;

  void setPin(bool on);
  void startPattern(BuzzerPattern p);
  void updateError(unsigned long now);
  void updateConfirm(unsigned long now, uint8_t shortsBeforePause, uint8_t shortsAfterPause,
                     bool afterIsLong);
  void updateDelayEscalation(unsigned long now);
  void updatePerimeter(unsigned long now);

public:
  BuzzerController();

  void begin();
  void loop();

  bool isBusy() const { return busy; }
  bool isPlayingDelay() const { return pattern == BUZZ_DELAY_ESCALATION; }

  void playError();
  void playConfirmZone1On();
  void playConfirmZone1Off();
  void playConfirmZone2On();
  void playConfirmZone2Off();
  void playConfirmCatOn();
  void playConfirmCatOff();
  void playDelayEscalation();
  void playPerimeterAlert();
  void stop();
};

#endif
