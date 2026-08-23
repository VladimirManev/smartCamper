#include "AlarmButtonHandler.h"
#include "Config.h"

AlarmButtonHandler::AlarmButtonHandler()
  : lastRaw(false),
    debouncedPressed(false),
    lastDebounceTime(0),
    pressStart(0),
    lastReleaseTime(0),
    shortTapCount(0),
    holdConsumed(false),
    waitingForHold(false),
    pendingAction(BUTTON_ACTION_NONE) {}

void AlarmButtonHandler::begin() {
  pinMode(ALARM_BUTTON_PIN, INPUT_PULLUP);
}

void AlarmButtonHandler::resetSequence() {
  shortTapCount = 0;
  waitingForHold = false;
  // holdConsumed cleared on release after a completed hold
}

void AlarmButtonHandler::loop() {
  unsigned long now = millis();
  bool rawPressed = (digitalRead(ALARM_BUTTON_PIN) == LOW);

  if (rawPressed != lastRaw) {
    lastDebounceTime = now;
  }
  lastRaw = rawPressed;

  if (now - lastDebounceTime < BUTTON_DEBOUNCE_MS) {
    return;
  }

  if (rawPressed == debouncedPressed) {
    // same state — check hold while pressed
    if (debouncedPressed && waitingForHold && !holdConsumed) {
      if (now - pressStart >= BUTTON_HOLD_MIN_MS) {
        holdConsumed = true;
        if (shortTapCount == 1) {
          pendingAction = BUTTON_ACTION_ZONE1_TOGGLE;
        } else if (shortTapCount == 2) {
          pendingAction = BUTTON_ACTION_ZONE2_TOGGLE;
        } else if (shortTapCount == 3) {
          pendingAction = BUTTON_ACTION_CAT_TOGGLE;
        } else {
          pendingAction = BUTTON_ACTION_ERROR;
        }
        shortTapCount = 0;
        waitingForHold = false;
      }
    }

    // timeout: gap between taps too long without starting hold press
    if (!debouncedPressed && shortTapCount > 0 && !waitingForHold) {
      if (now - lastReleaseTime > BUTTON_TAP_GAP_MAX_MS) {
        pendingAction = BUTTON_ACTION_ERROR;
        resetSequence();
        holdConsumed = false;
      }
    }
    return;
  }

  // state change
  debouncedPressed = rawPressed;

  if (debouncedPressed) {
    // press down
    pressStart = now;
    holdConsumed = false;

    if (shortTapCount > 0 && (now - lastReleaseTime) > BUTTON_TAP_GAP_MAX_MS) {
      pendingAction = BUTTON_ACTION_ERROR;
      resetSequence();
      pressStart = now;
    }

    waitingForHold = true;
  } else {
    // release
    unsigned long pressDuration = now - pressStart;
    lastReleaseTime = now;

    if (holdConsumed) {
      holdConsumed = false;
      waitingForHold = false;
      return;
    }

    if (pressDuration < BUTTON_SHORT_MAX_MS) {
      shortTapCount++;
      waitingForHold = false;
      if (shortTapCount > 3) {
        pendingAction = BUTTON_ACTION_ERROR;
        resetSequence();
      }
    } else if (pressDuration < BUTTON_HOLD_MIN_MS) {
      pendingAction = BUTTON_ACTION_ERROR;
      resetSequence();
    } else {
      waitingForHold = false;
      resetSequence();
    }
  }
}

AlarmButtonAction AlarmButtonHandler::consumeAction() {
  AlarmButtonAction a = pendingAction;
  pendingAction = BUTTON_ACTION_NONE;
  return a;
}
