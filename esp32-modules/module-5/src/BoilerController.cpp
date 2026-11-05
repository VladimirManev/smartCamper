// Boiler thermostat
// 12-bit DS18B20 conversion is ~750 ms. Both buses convert together and the
// wait is split across loop() so buttons and MQTT keep running.
// The electrode pull-up is on only for a few milliseconds per sample so the
// pin does not feed current through the water between readings.

#include "BoilerController.h"
#include "ApplianceManager.h"
#include "MQTTManager.h"
#include <ArduinoJson.h>
#include <math.h>

BoilerController::BoilerController(MQTTManager* mqtt, ApplianceManager* appMgr)
  : mqttManager(mqtt),
    applianceManager(appMgr),
    bottomWire(BOILER_TEMP_BOTTOM_PIN),
    topWire(BOILER_TEMP_TOP_PIN),
    bottomSensors(&bottomWire),
    topSensors(&topWire),
    enabled(false),
    elementOn(false),
    enabledChanged(false),
    setpoint(BOILER_SETPOINT_DEFAULT),
    hasFull(false),
    full(false),
    error(ERROR_NONE),
    burstActive(false),
    waitingConversion(false),
    immediateBurst(false),
    burstIndex(0),
    conversionStartedMs(0),
    lastBurstMs(0),
    lastPayload("") {
  bottom.reading = 0;
  bottom.hasReading = false;
  bottom.failCount = 0;
  top.reading = 0;
  top.hasReading = false;
  top.failCount = 0;
  clearSamples(bottom);
  clearSamples(top);
}

void BoilerController::begin() {
  pinMode(RELAY_PIN_4, OUTPUT);
  digitalWrite(RELAY_PIN_4, LOW);
  pinMode(BOILER_ELECTRODE_PIN, INPUT);

  bottomSensors.begin();
  topSensors.begin();
  bottomSensors.setResolution(12);
  topSensors.setResolution(12);
  bottomSensors.setWaitForConversion(false);
  topSensors.setWaitForConversion(false);

  hasFull = true;
  full = readElectrodeFull();

  if (DEBUG_SERIAL) {
    Serial.println("♨️ Boiler controller ready");
    Serial.println("   Electrode GPIO " + String(BOILER_ELECTRODE_PIN) +
                   " full=" + String(full ? "yes" : "no"));
    Serial.println("   Bottom GPIO " + String(BOILER_TEMP_BOTTOM_PIN) +
                   " devices=" + String(bottomSensors.getDeviceCount()));
    Serial.println("   Top GPIO " + String(BOILER_TEMP_TOP_PIN) +
                   " devices=" + String(topSensors.getDeviceCount()));
  }
}

void BoilerController::loop() {
  unsigned long now = millis();

  if (!burstActive) {
    bool due = immediateBurst || lastBurstMs == 0 ||
               (now - lastBurstMs >= BOILER_TEMP_INTERVAL_MS);
    if (due) {
      startBurst(now);
    }
    return;
  }

  if (!waitingConversion) {
    bottomSensors.requestTemperatures();
    topSensors.requestTemperatures();
    waitingConversion = true;
    conversionStartedMs = now;
    return;
  }

  if (now - conversionStartedMs < BOILER_TEMP_CONVERSION_MS) {
    return;
  }

  bottom.samples[burstIndex] = readCelsius(bottomSensors);
  top.samples[burstIndex] = readCelsius(topSensors);
  burstIndex++;
  waitingConversion = false;

  if (burstIndex < BOILER_TEMP_BURST_COUNT) {
    return;
  }

  finishBurst(now);
}

void BoilerController::requestOn() {
  error = ERROR_NONE;
  bottom.failCount = 0;
  top.failCount = 0;
  setEnabled(true);
  immediateBurst = true;
  if (bottom.hasReading && top.hasReading) {
    applyControl();
  }
  publish(true);
}

void BoilerController::requestOff() {
  setEnabled(false);
  setElement(false);
  error = ERROR_NONE;
  publish(true);
}

void BoilerController::onButton() {
  if (enabled) {
    requestOff();
  } else {
    requestOn();
  }
}

void BoilerController::setSetpoint(int value) {
  if (value < BOILER_SETPOINT_MIN || value > BOILER_SETPOINT_MAX) {
    return;
  }
  if (value == setpoint) {
    return;
  }
  setpoint = value;
  if (enabled && error == ERROR_NONE && bottom.hasReading && top.hasReading) {
    applyControl();
  }
  publish(true);
}

void BoilerController::onForceUpdate() {
  bool isFull = readElectrodeFull();
  hasFull = true;
  full = isFull;
  if (elementOn && !isFull) {
    trip(ERROR_EMPTY);
    publish(true);
    return;
  }
  if (isFull && error == ERROR_EMPTY) {
    error = ERROR_NONE;
  }
  publish(true);
}

void BoilerController::startBurst(unsigned long now) {
  (void)now;
  burstActive = true;
  waitingConversion = false;
  burstIndex = 0;
  immediateBurst = false;
  clearSamples(bottom);
  clearSamples(top);
}

void BoilerController::finishBurst(unsigned long now) {
  burstActive = false;
  waitingConversion = false;
  lastBurstMs = now;

  int bottomRounded = 0;
  int topRounded = 0;
  bool bottomOk = median(bottom.samples, &bottomRounded);
  bool topOk = median(top.samples, &topRounded);

  if (bottomOk) {
    bottom.failCount = 0;
    bottom.hasReading = true;
    bottom.reading = bottomRounded;
  } else if (bottom.failCount < BOILER_TEMP_MAX_FAILED_BURSTS) {
    bottom.failCount++;
  }

  if (topOk) {
    top.failCount = 0;
    top.hasReading = true;
    top.reading = topRounded;
  } else if (top.failCount < BOILER_TEMP_MAX_FAILED_BURSTS) {
    top.failCount++;
  }

  bool bottomHot = bottomOk && bottom.reading >= BOILER_OVERTEMP_C;
  bool topHot = topOk && top.reading >= BOILER_OVERTEMP_C;
  if (bottomHot || topHot) {
    trip(ERROR_OVERTEMP);
  } else if (bottom.failCount >= BOILER_TEMP_MAX_FAILED_BURSTS ||
             top.failCount >= BOILER_TEMP_MAX_FAILED_BURSTS) {
    trip(ERROR_SENSOR);
  } else {
    applyControl();
  }

  publish(false);
}

void BoilerController::applyControl() {
  if (!enabled || error != ERROR_NONE) {
    setElement(false);
    return;
  }
  if (!bottom.hasReading || !top.hasReading) {
    setElement(false);
    return;
  }
  if (bottom.reading >= BOILER_OVERTEMP_C || top.reading >= BOILER_OVERTEMP_C) {
    trip(ERROR_OVERTEMP);
    return;
  }

  if (elementOn) {
    if (bottom.reading >= setpoint) {
      setElement(false);
    }
    return;
  }

  if (bottom.reading <= setpoint - BOILER_HYSTERESIS) {
    bool isFull = readElectrodeFull();
    hasFull = true;
    full = isFull;
    if (!isFull) {
      trip(ERROR_EMPTY);
      return;
    }
    setElement(true);
  }
}

void BoilerController::trip(Error code) {
  error = code;
  setEnabled(false);
  if (code == ERROR_SENSOR) {
    bottom.hasReading = false;
    top.hasReading = false;
  }
  setElement(false);
  if (DEBUG_SERIAL) {
    Serial.println("♨️ Boiler stopped: " + String(errorName()));
  }
}

void BoilerController::setEnabled(bool on) {
  if (enabled == on) {
    return;
  }
  enabled = on;
  enabledChanged = true;
}

void BoilerController::setElement(bool on) {
  if (elementOn == on) {
    return;
  }
  elementOn = on;
  digitalWrite(RELAY_PIN_4, on ? HIGH : LOW);
  if (DEBUG_SERIAL) {
    Serial.println("♨️ Boiler element " + String(on ? "ON" : "OFF"));
  }
}

void BoilerController::publish(bool force) {
  if (mqttManager != nullptr && mqttManager->isMQTTConnected()) {
    StaticJsonDocument<384> doc;
    doc["enabled"] = enabled;
    doc["relay"] = elementOn;
    doc["setpoint"] = setpoint;
    if (top.hasReading) {
      doc["top"] = top.reading;
    } else {
      doc["top"] = nullptr;
    }
    if (bottom.hasReading) {
      doc["bottom"] = bottom.reading;
    } else {
      doc["bottom"] = nullptr;
    }
    if (hasFull) {
      doc["full"] = full;
    } else {
      doc["full"] = nullptr;
    }
    if (error == ERROR_NONE) {
      doc["error"] = nullptr;
    } else {
      doc["error"] = errorName();
    }

    String payload;
    serializeJson(doc, payload);
    if (force || payload != lastPayload) {
      lastPayload = payload;
      String topic = String(MQTT_TOPIC_SENSORS) + MODULE_ID + "/boiler";
      mqttManager->publishRaw(topic, payload);
    }
  }

  if (enabledChanged) {
    enabledChanged = false;
    if (applianceManager != nullptr) {
      applianceManager->publishRelayStatus();
    }
  }
}

bool BoilerController::readElectrodeFull() {
  int waterHits = 0;
  for (int i = 0; i < 3; i++) {
    pinMode(BOILER_ELECTRODE_PIN, INPUT_PULLUP);
    delay(5);
    if (digitalRead(BOILER_ELECTRODE_PIN) == LOW) {
      waterHits++;
    }
    pinMode(BOILER_ELECTRODE_PIN, INPUT);
    digitalWrite(BOILER_ELECTRODE_PIN, LOW);
    delay(2);
  }
  return waterHits >= 2;
}

float BoilerController::readCelsius(DallasTemperature& sensors) {
  float temp = sensors.getTempCByIndex(0);
  if (isnan(temp) || temp == DEVICE_DISCONNECTED_C) {
    return NAN;
  }
  if (temp < BOILER_TEMP_MIN_VALID || temp > BOILER_TEMP_MAX_VALID) {
    return NAN;
  }
  return temp;
}

bool BoilerController::median(const float* samples, int* roundedOut) {
  float valid[BOILER_TEMP_BURST_COUNT];
  int count = 0;
  for (int i = 0; i < BOILER_TEMP_BURST_COUNT; i++) {
    if (!isnan(samples[i])) {
      valid[count++] = samples[i];
    }
  }
  if (count < 2) {
    return false;
  }

  float result;
  if (count == 2) {
    result = (valid[0] + valid[1]) / 2.0f;
  } else {
    if (valid[0] > valid[1]) {
      float swap = valid[0];
      valid[0] = valid[1];
      valid[1] = swap;
    }
    if (valid[1] > valid[2]) {
      float swap = valid[1];
      valid[1] = valid[2];
      valid[2] = swap;
    }
    if (valid[0] > valid[1]) {
      float swap = valid[0];
      valid[0] = valid[1];
      valid[1] = swap;
    }
    result = valid[1];
  }

  *roundedOut = (int)lround(result);
  return true;
}

void BoilerController::clearSamples(TempChannel& channel) {
  for (int i = 0; i < BOILER_TEMP_BURST_COUNT; i++) {
    channel.samples[i] = NAN;
  }
}

const char* BoilerController::errorName() const {
  if (error == ERROR_EMPTY) {
    return "empty";
  }
  if (error == ERROR_SENSOR) {
    return "sensor";
  }
  if (error == ERROR_OVERTEMP) {
    return "overtemp";
  }
  return "";
}
