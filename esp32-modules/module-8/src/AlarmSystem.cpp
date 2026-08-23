#include "AlarmSystem.h"
#include <string.h>

const uint8_t AlarmSystem::perimeterPins[NUM_PERIMETER_PIRS] = {
  PERIMETER_PIR_FRONT,
  PERIMETER_PIR_REAR,
  PERIMETER_PIR_LEFT_FRONT,
  PERIMETER_PIR_LEFT_REAR,
  PERIMETER_PIR_RIGHT_FRONT,
  PERIMETER_PIR_RIGHT_REAR
};

const char* AlarmSystem::perimeterNames[NUM_PERIMETER_PIRS] = {
  "front", "rear", "left_front", "left_rear", "right_front", "right_rear"
};

AlarmSystem::AlarmSystem(BuzzerController* buzzerCtrl)
  : buzzer(buzzerCtrl),
    zone1Phase(Z1_IDLE),
    zone1IgnoreInteriorPir(false),
    zone2Armed(false),
    sirenOn(false),
    smokeOn(false),
    zone1LedOn(false),
    zone1LedLastToggle(0),
    phaseStartMs(0),
    sirenStartMs(0),
    smokeStartMs(0),
    lastPerimeterAlertMs(0),
    spareOpen(false),
    lastSpareOpen(false),
    spareLatchedOpen(false),
    interiorPir(false),
    statusDirty(true),
    waitingConfirmThenExit(false),
    pendingArmIsCat(false) {
  for (int i = 0; i < NUM_PERIMETER_PIRS; i++) {
    perimeterPir[i] = false;
    lastPerimeterPir[i] = false;
  }
}

void AlarmSystem::begin() {
  button.begin();

  pinMode(SPARE_CONTACT_PIN, INPUT_PULLUP);
  pinMode(INTERIOR_PIR_PIN, INPUT);
  for (int i = 0; i < NUM_PERIMETER_PIRS; i++) {
    pinMode(perimeterPins[i], INPUT);
  }

  pinMode(SIREN_RELAY_PIN, OUTPUT);
  pinMode(SMOKE_RELAY_PIN, OUTPUT);
  pinMode(ZONE1_LED_PIN, OUTPUT);
  setSiren(false);
  setSmoke(false);
  digitalWrite(ZONE1_LED_PIN, LOW);

  readInputs();
  lastSpareOpen = spareOpen;
  if (spareOpen) {
    spareLatchedOpen = true;  // boot with open: ignore until closed then reopened
  }

  if (DEBUG_SERIAL) {
    Serial.println("AlarmSystem ready");
  }
}

void AlarmSystem::readInputs() {
  spareOpen = (digitalRead(SPARE_CONTACT_PIN) == LOW);
  interiorPir = (digitalRead(INTERIOR_PIR_PIN) == HIGH);
  for (int i = 0; i < NUM_PERIMETER_PIRS; i++) {
    perimeterPir[i] = (digitalRead(perimeterPins[i]) == HIGH);
  }
}

bool AlarmSystem::getPerimeterPir(uint8_t i) const {
  if (i >= NUM_PERIMETER_PIRS) {
    return false;
  }
  return perimeterPir[i];
}

const char* AlarmSystem::phaseToString() const {
  switch (zone1Phase) {
    case Z1_IDLE: return "idle";
    case Z1_EXIT_DELAY: return "exit_delay";
    case Z1_ARMED: return "armed";
    case Z1_ENTRY_DELAY: return "entry_delay";
    case Z1_ALARM: return "alarm";
    default: return "idle";
  }
}

bool AlarmSystem::consumeStatusDirty() {
  if (!statusDirty) {
    return false;
  }
  statusDirty = false;
  return true;
}

void AlarmSystem::setSiren(bool on) {
  if (sirenOn != on) {
    sirenOn = on;
    digitalWrite(SIREN_RELAY_PIN, on ? HIGH : LOW);
    statusDirty = true;
  }
}

void AlarmSystem::setSmoke(bool on) {
  if (smokeOn != on) {
    smokeOn = on;
    digitalWrite(SMOKE_RELAY_PIN, on ? HIGH : LOW);
    statusDirty = true;
  }
}

void AlarmSystem::stopAlarmOutputs() {
  setSiren(false);
  setSmoke(false);
}

void AlarmSystem::updateZone1Led() {
  bool shouldBlink = (zone1Phase != Z1_IDLE);
  if (!shouldBlink) {
    if (zone1LedOn) {
      zone1LedOn = false;
      digitalWrite(ZONE1_LED_PIN, LOW);
    }
    return;
  }

  unsigned long now = millis();
  if (now - zone1LedLastToggle >= ZONE1_LED_BLINK_MS) {
    zone1LedLastToggle = now;
    zone1LedOn = !zone1LedOn;
    digitalWrite(ZONE1_LED_PIN, zone1LedOn ? HIGH : LOW);
  }
}

void AlarmSystem::startExitDelay() {
  zone1Phase = Z1_EXIT_DELAY;
  phaseStartMs = millis();
  if (buzzer) {
    buzzer->playDelayEscalation();
  }
  statusDirty = true;
}

void AlarmSystem::startEntryDelay() {
  zone1Phase = Z1_ENTRY_DELAY;
  phaseStartMs = millis();
  if (buzzer) {
    buzzer->playDelayEscalation();
  }
  statusDirty = true;
}

void AlarmSystem::startAlarm() {
  zone1Phase = Z1_ALARM;
  phaseStartMs = millis();
  sirenStartMs = millis();
  smokeStartMs = 0;
  if (buzzer) {
    buzzer->stop();
  }
  setSiren(true);
  setSmoke(false);
  statusDirty = true;
}

void AlarmSystem::disarmZone1() {
  Zone1Phase prev = zone1Phase;
  bool wasCat = zone1IgnoreInteriorPir;
  zone1Phase = Z1_IDLE;
  zone1IgnoreInteriorPir = false;
  waitingConfirmThenExit = false;
  stopAlarmOutputs();
  if (buzzer) {
    buzzer->stop();
    if (wasCat) {
      buzzer->playConfirmCatOff();
    } else {
      buzzer->playConfirmZone1Off();
    }
  }
  // latch spare if still open so re-arm doesn't instantly trip
  if (spareOpen) {
    spareLatchedOpen = true;
  }
  statusDirty = true;
  (void)prev;
}

bool AlarmSystem::armZone1(bool ignoreInteriorPir) {
  if (zone1Phase != Z1_IDLE) {
    // Option C: already armed — reject other mode / re-arm
    if (zone1IgnoreInteriorPir != ignoreInteriorPir) {
      if (buzzer) {
        buzzer->playError();
      }
      return false;
    }
    // same mode already armed — treat as no-op / error
    if (buzzer) {
      buzzer->playError();
    }
    return false;
  }

  zone1IgnoreInteriorPir = ignoreInteriorPir;
  pendingArmIsCat = ignoreInteriorPir;
  waitingConfirmThenExit = true;
  if (buzzer) {
    if (ignoreInteriorPir) {
      buzzer->playConfirmCatOn();
    } else {
      buzzer->playConfirmZone1On();
    }
  }
  // Phase stays idle until confirm finishes, then exit delay
  statusDirty = true;
  return true;
}

bool AlarmSystem::disarmZone1WithPin(const char* pin) {
  if (pin == nullptr || strcmp(pin, ALARM_DISARM_PIN) != 0) {
    if (buzzer) {
      buzzer->playError();
    }
    statusDirty = true;
    return false;
  }
  if (zone1Phase == Z1_IDLE && !waitingConfirmThenExit) {
    return true;  // already off
  }
  waitingConfirmThenExit = false;
  disarmZone1();
  return true;
}

bool AlarmSystem::armZone2() {
  if (zone2Armed) {
    return true;
  }
  zone2Armed = true;
  lastPerimeterAlertMs = 0;
  if (buzzer) {
    buzzer->playConfirmZone2On();
  }
  statusDirty = true;
  return true;
}

bool AlarmSystem::disarmZone2() {
  if (!zone2Armed) {
    return true;
  }
  zone2Armed = false;
  if (buzzer) {
    buzzer->playConfirmZone2Off();
  }
  statusDirty = true;
  return true;
}

void AlarmSystem::handleButtonAction(AlarmButtonAction action) {
  switch (action) {
    case BUTTON_ACTION_ZONE1_TOGGLE:
      if (zone1Phase == Z1_IDLE && !waitingConfirmThenExit) {
        armZone1(false);
      } else if (zone1Phase != Z1_IDLE || waitingConfirmThenExit) {
        if (zone1IgnoreInteriorPir || pendingArmIsCat) {
          // armed/arming as cat — normal toggle is conflict
          if (buzzer) {
            buzzer->playError();
          }
        } else {
          waitingConfirmThenExit = false;
          disarmZone1();
        }
      }
      break;

    case BUTTON_ACTION_CAT_TOGGLE:
      if (zone1Phase == Z1_IDLE && !waitingConfirmThenExit) {
        armZone1(true);
      } else if (zone1Phase != Z1_IDLE || waitingConfirmThenExit) {
        if (!zone1IgnoreInteriorPir && !pendingArmIsCat) {
          if (buzzer) {
            buzzer->playError();
          }
        } else {
          waitingConfirmThenExit = false;
          disarmZone1();
        }
      }
      break;

    case BUTTON_ACTION_ZONE2_TOGGLE:
      if (zone2Armed) {
        disarmZone2();
      } else {
        armZone2();
      }
      break;

    case BUTTON_ACTION_ERROR:
      if (buzzer) {
        buzzer->playError();
      }
      break;

    default:
      break;
  }
}

void AlarmSystem::processZone1Sensors() {
  if (zone1Phase != Z1_ARMED) {
    lastSpareOpen = spareOpen;
    return;
  }

  // Clear latch when spare closes
  if (!spareOpen) {
    spareLatchedOpen = false;
  }

  bool spareEdge = spareOpen && !lastSpareOpen && !spareLatchedOpen;
  bool pirTrip = !zone1IgnoreInteriorPir && interiorPir;

  if (spareEdge || pirTrip) {
    if (spareEdge) {
      spareLatchedOpen = true;
    }
    startEntryDelay();
  }

  lastSpareOpen = spareOpen;
}

void AlarmSystem::processPerimeter() {
  if (!zone2Armed) {
    for (int i = 0; i < NUM_PERIMETER_PIRS; i++) {
      lastPerimeterPir[i] = perimeterPir[i];
    }
    return;
  }

  bool anyMotion = false;
  bool rising = false;
  for (int i = 0; i < NUM_PERIMETER_PIRS; i++) {
    if (perimeterPir[i]) {
      anyMotion = true;
    }
    if (perimeterPir[i] && !lastPerimeterPir[i]) {
      rising = true;
    }
    lastPerimeterPir[i] = perimeterPir[i];
  }

  if (!anyMotion) {
    return;
  }

  // Suppress while delay buzzer is active
  if (buzzer && buzzer->isPlayingDelay()) {
    return;
  }

  unsigned long now = millis();
  bool dueRepeat = (lastPerimeterAlertMs > 0) && (now - lastPerimeterAlertMs >= PERIMETER_REPEAT_MS);

  if (rising || dueRepeat) {
    if (buzzer) {
      buzzer->playPerimeterAlert();
    }
    lastPerimeterAlertMs = now;
    statusDirty = true;
  }
}

void AlarmSystem::updateExitEntryDelay() {
  if (zone1Phase != Z1_EXIT_DELAY && zone1Phase != Z1_ENTRY_DELAY) {
    return;
  }

  unsigned long elapsed = millis() - phaseStartMs;
  unsigned long limit = (zone1Phase == Z1_EXIT_DELAY) ? EXIT_DELAY_MS : ENTRY_DELAY_MS;

  if (elapsed < limit) {
    return;
  }

  if (zone1Phase == Z1_EXIT_DELAY) {
    if (buzzer) {
      buzzer->stop();
    }
    zone1Phase = Z1_ARMED;
    statusDirty = true;

    // Spec: open spare or interior motion after exit -> immediate entry delay
    bool trip = false;
    if (spareOpen) {
      trip = true;
      spareLatchedOpen = true;
    }
    if (!zone1IgnoreInteriorPir && interiorPir) {
      trip = true;
    }
    if (trip) {
      startEntryDelay();
    }
    lastSpareOpen = spareOpen;
    return;
  }

  // entry delay finished -> alarm
  startAlarm();
}

void AlarmSystem::updateAlarmPhase() {
  if (zone1Phase != Z1_ALARM) {
    return;
  }

  unsigned long now = millis();
  unsigned long sinceSiren = now - sirenStartMs;

  // Smoke: start 10s after siren, run 1 minute
  if (!smokeOn && sinceSiren >= SMOKE_START_AFTER_SIREN_MS &&
      sinceSiren < (SMOKE_START_AFTER_SIREN_MS + SMOKE_DURATION_MS)) {
    setSmoke(true);
    smokeStartMs = now;
  }
  if (smokeOn && smokeStartMs > 0 &&
      (now - smokeStartMs >= SMOKE_DURATION_MS)) {
    setSmoke(false);
  }
  // Also stop smoke if past window from siren start
  if (smokeOn && sinceSiren >= (SMOKE_START_AFTER_SIREN_MS + SMOKE_DURATION_MS)) {
    setSmoke(false);
  }

  if (sinceSiren >= SIREN_DURATION_MS) {
    stopAlarmOutputs();
    zone1Phase = Z1_ARMED;
    statusDirty = true;
    // Keep spare latched if still open
    if (spareOpen) {
      spareLatchedOpen = true;
    }
    lastSpareOpen = spareOpen;
  }
}

void AlarmSystem::loop() {
  button.loop();
  AlarmButtonAction action = button.consumeAction();
  if (action != BUTTON_ACTION_NONE) {
    handleButtonAction(action);
  }

  // After confirmation finishes, start exit delay
  if (waitingConfirmThenExit && buzzer && !buzzer->isBusy()) {
    waitingConfirmThenExit = false;
    startExitDelay();
  }

  readInputs();

  // Publish when inputs change
  static bool prevSpare = false;
  static bool prevPir = false;
  static bool prevPerim[NUM_PERIMETER_PIRS] = {false};
  if (spareOpen != prevSpare || interiorPir != prevPir) {
    statusDirty = true;
    prevSpare = spareOpen;
    prevPir = interiorPir;
  }
  for (int i = 0; i < NUM_PERIMETER_PIRS; i++) {
    if (perimeterPir[i] != prevPerim[i]) {
      statusDirty = true;
      prevPerim[i] = perimeterPir[i];
    }
  }

  processZone1Sensors();
  processPerimeter();
  updateExitEntryDelay();
  updateAlarmPhase();
  updateZone1Led();
}
