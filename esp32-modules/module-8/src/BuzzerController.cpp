#include "BuzzerController.h"
#include "Config.h"

BuzzerController::BuzzerController()
  : pattern(BUZZ_IDLE),
    patternStart(0),
    nextToggleAt(0),
    pinHigh(false),
    busy(false),
    step(0),
    shortBeepRemaining(0) {}

void BuzzerController::begin() {
  pinMode(BUZZER_PIN, OUTPUT);
  setPin(false);
}

void BuzzerController::setPin(bool on) {
  digitalWrite(BUZZER_PIN, on ? HIGH : LOW);
  pinHigh = on;
}

void BuzzerController::stop() {
  pattern = BUZZ_IDLE;
  busy = false;
  step = 0;
  setPin(false);
}

void BuzzerController::startPattern(BuzzerPattern p) {
  pattern = p;
  patternStart = millis();
  nextToggleAt = patternStart;
  busy = true;
  step = 0;
  shortBeepRemaining = 0;
  setPin(false);
}

void BuzzerController::playError() {
  startPattern(BUZZ_ERROR);
  setPin(true);
  nextToggleAt = millis() + ERROR_BEEP_MS;
}

void BuzzerController::playConfirmZone1On() { startPattern(BUZZ_CONFIRM_Z1_ON); }
void BuzzerController::playConfirmZone1Off() { startPattern(BUZZ_CONFIRM_Z1_OFF); }
void BuzzerController::playConfirmZone2On() { startPattern(BUZZ_CONFIRM_Z2_ON); }
void BuzzerController::playConfirmZone2Off() { startPattern(BUZZ_CONFIRM_Z2_OFF); }
void BuzzerController::playConfirmCatOn() { startPattern(BUZZ_CONFIRM_CAT_ON); }
void BuzzerController::playConfirmCatOff() { startPattern(BUZZ_CONFIRM_CAT_OFF); }

void BuzzerController::playDelayEscalation() {
  startPattern(BUZZ_DELAY_ESCALATION);
  setPin(true);
  nextToggleAt = millis() + BEEP_SHORT_MS;
}

void BuzzerController::playPerimeterAlert() {
  if (busy && pattern == BUZZ_DELAY_ESCALATION) {
    return;  // suppressed while delay indication runs
  }
  if (busy && pattern == BUZZ_PERIMETER_ALERT) {
    return;  // do not restart mid-alert (noisy pins)
  }
  startPattern(BUZZ_PERIMETER_ALERT);
  shortBeepRemaining = PERIMETER_BEEP_COUNT;
  setPin(true);
  nextToggleAt = millis() + BEEP_SHORT_MS;
}

void BuzzerController::loop() {
  if (!busy) {
    return;
  }

  unsigned long now = millis();

  switch (pattern) {
    case BUZZ_ERROR:
      updateError(now);
      break;
    case BUZZ_CONFIRM_Z1_ON:
      updateConfirm(now, 1, 0, true);
      break;
    case BUZZ_CONFIRM_Z1_OFF:
      updateConfirm(now, 1, 3, false);
      break;
    case BUZZ_CONFIRM_Z2_ON:
      updateConfirm(now, 2, 0, true);
      break;
    case BUZZ_CONFIRM_Z2_OFF:
      updateConfirm(now, 2, 3, false);
      break;
    case BUZZ_CONFIRM_CAT_ON:
      updateConfirm(now, 3, 0, true);
      break;
    case BUZZ_CONFIRM_CAT_OFF:
      updateConfirm(now, 3, 3, false);
      break;
    case BUZZ_DELAY_ESCALATION:
      updateDelayEscalation(now);
      break;
    case BUZZ_PERIMETER_ALERT:
      updatePerimeter(now);
      break;
    default:
      stop();
      break;
  }
}

void BuzzerController::updateError(unsigned long now) {
  if (now >= nextToggleAt) {
    stop();
  }
}

// shortsBeforePause: N short beeps, then CONFIRM_PAUSE, then either 1 long or shortsAfterPause shorts
void BuzzerController::updateConfirm(unsigned long now, uint8_t shortsBeforePause,
                                     uint8_t shortsAfterPause, bool afterIsLong) {
  // step encoding:
  // 0..2*(shortsBefore)-1 : short on/off pairs
  // then pause
  // then after pattern
  if (now < nextToggleAt) {
    return;
  }

  uint8_t beforeSteps = shortsBeforePause * 2;

  if (step < beforeSteps) {
    bool turnOn = (step % 2) == 0;
    setPin(turnOn);
    nextToggleAt = now + (turnOn ? BEEP_SHORT_MS : BEEP_GAP_MS);
    step++;
    return;
  }

  if (step == beforeSteps) {
    // start pause (buzzer off)
    setPin(false);
    nextToggleAt = now + CONFIRM_PAUSE_MS;
    step++;
    return;
  }

  // after pause
  uint8_t afterStart = beforeSteps + 1;
  if (afterIsLong) {
    if (step == afterStart) {
      setPin(true);
      nextToggleAt = now + BEEP_LONG_MS;
      step++;
      return;
    }
    stop();
    return;
  }

  // N short after pause
  uint8_t afterSteps = shortsAfterPause * 2;
  uint8_t afterIndex = step - afterStart;
  if (afterIndex < afterSteps) {
    bool turnOn = (afterIndex % 2) == 0;
    setPin(turnOn);
    nextToggleAt = now + (turnOn ? BEEP_SHORT_MS : BEEP_GAP_MS);
    step++;
    return;
  }
  stop();
}

void BuzzerController::updateDelayEscalation(unsigned long now) {
  unsigned long elapsed = now - patternStart;

  if (elapsed >= (DELAY_PHASE1_MS + DELAY_PHASE2_MS + DELAY_PHASE3_MS)) {
    stop();
    return;
  }

  if (now < nextToggleAt) {
    return;
  }

  if (elapsed < DELAY_PHASE1_MS) {
    // 1 beep per second: short on, then off until next second
    if (!pinHigh) {
      setPin(true);
      nextToggleAt = now + BEEP_SHORT_MS;
    } else {
      setPin(false);
      unsigned long nextBeep = patternStart + ((elapsed / 1000) + 1) * 1000UL;
      nextToggleAt = nextBeep;
    }
    return;
  }

  if (elapsed < DELAY_PHASE1_MS + DELAY_PHASE2_MS) {
    // phase 2: 2 beeps per second
    if (!pinHigh) {
      setPin(true);
      nextToggleAt = now + BEEP_SHORT_MS;
    } else {
      setPin(false);
      nextToggleAt = now + (500 - BEEP_SHORT_MS);
    }
    return;
  }

  // phase 3: 6 beeps per second
  if (!pinHigh) {
    setPin(true);
    nextToggleAt = now + BEEP_SHORT_MS;
  } else {
    setPin(false);
    nextToggleAt = now + (DELAY_PHASE3_PERIOD_MS - BEEP_SHORT_MS);
  }
}

void BuzzerController::updatePerimeter(unsigned long now) {
  if (now < nextToggleAt) {
    return;
  }

  if (pinHigh) {
    setPin(false);
    shortBeepRemaining--;
    if (shortBeepRemaining == 0) {
      stop();
      return;
    }
    nextToggleAt = now + BEEP_GAP_MS;
  } else {
    setPin(true);
    nextToggleAt = now + BEEP_SHORT_MS;
  }
}
