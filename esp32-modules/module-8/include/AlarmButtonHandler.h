// BOOT button sequence: N short taps + hold >= 3s

#ifndef ALARM_BUTTON_HANDLER_H
#define ALARM_BUTTON_HANDLER_H

#include <Arduino.h>

enum AlarmButtonAction {
  BUTTON_ACTION_NONE = 0,
  BUTTON_ACTION_ZONE1_TOGGLE,
  BUTTON_ACTION_ZONE2_TOGGLE,
  BUTTON_ACTION_CAT_TOGGLE,
  BUTTON_ACTION_ERROR
};

class AlarmButtonHandler {
private:
  bool lastRaw;
  bool debouncedPressed;
  unsigned long lastDebounceTime;
  unsigned long pressStart;
  unsigned long lastReleaseTime;
  uint8_t shortTapCount;
  bool holdConsumed;
  bool waitingForHold;
  AlarmButtonAction pendingAction;

  void resetSequence();

public:
  AlarmButtonHandler();

  void begin();
  void loop();

  // Returns action once, then clears
  AlarmButtonAction consumeAction();
};

#endif
