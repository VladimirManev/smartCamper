// Floor Heating Temperature Sensor Implementation
// Burst of DS18B20 readings → median every HEATING_TEMP_INTERVAL_MS (and on ON / relay change)

#include "FloorHeatingSensor.h"
#include "FloorHeatingController.h"
#include "FloorHeatingManager.h"
#include <Arduino.h>
#include <math.h>

unsigned long FloorHeatingSensor::globalRelaySettleUntil = 0;

FloorHeatingSensor::FloorHeatingSensor(MQTTManager* mqtt, uint8_t circleIndex, uint8_t pin)
  : oneWire(pin), sensors(&oneWire) {
  if (mqtt == nullptr && DEBUG_SERIAL) {
    Serial.println("❌ ERROR: FloorHeatingSensor: mqttManager cannot be nullptr!");
  }
  if (circleIndex >= NUM_HEATING_CIRCLES && DEBUG_SERIAL) {
    Serial.println("❌ ERROR: FloorHeatingSensor: circleIndex out of range!");
  }

  this->mqttManager = mqtt;
  this->circleIndex = circleIndex;
  this->pin = pin;
  this->lastTemperature = NAN;
  this->lastPublishedTemperature = NAN;
  this->forceUpdateRequested = false;
  this->immediateBurstRequested = false;
  this->lastMQTTState = false;
  this->lastKnownMode = CIRCLE_MODE_OFF;
  this->burstInProgress = false;
  this->conversionStarted = false;
  this->conversionStartTime = 0;
  this->lastBurstTime = 0;
  this->burstIndex = 0;
  this->failedBurstCount = 0;
  this->hasError = false;
  this->controller = nullptr;
  this->manager = nullptr;

  for (int i = 0; i < HEATING_TEMP_BURST_COUNT; i++) {
    this->burstSamples[i] = NAN;
  }
}

void FloorHeatingSensor::begin() {
  sensors.begin();
  sensors.setResolution(12);
  // Non-blocking: requestTemperatures returns immediately; wait HEATING_TEMP_CONVERSION_MS before read
  sensors.setWaitForConversion(false);

  if (DEBUG_SERIAL) {
    Serial.println("🌡️ DS18B20 Floor Heating Sensor " + String(circleIndex) + " initialized");
    Serial.println("   GPIO pin: " + String(pin));
    int deviceCount = sensors.getDeviceCount();
    Serial.println("   Found " + String(deviceCount) + " DS18B20 device(s)");
    if (deviceCount == 0) {
      Serial.println("⚠️ WARNING: No DS18B20 sensors found on pin for circle " + String(circleIndex));
    }
  }
}

void FloorHeatingSensor::setController(FloorHeatingController* ctrl) {
  controller = ctrl;
}

void FloorHeatingSensor::setManager(FloorHeatingManager* mgr) {
  manager = mgr;
}

bool FloorHeatingSensor::isGlobalRelaySettling() {
  return millis() < globalRelaySettleUntil;
}

void FloorHeatingSensor::beginGlobalRelaySettle() {
  globalRelaySettleUntil = millis() + HEATING_RELAY_SETTLE_MS;
}

void FloorHeatingSensor::onRelayChanged() {
  abortBurst();
  immediateBurstRequested = true;
}

void FloorHeatingSensor::forceUpdate() {
  forceUpdateRequested = true;
  immediateBurstRequested = true;
}

void FloorHeatingSensor::clearMeasurementState() {
  abortBurst();
  lastTemperature = NAN;
  lastPublishedTemperature = NAN;
  failedBurstCount = 0;
  hasError = false;
  lastBurstTime = 0;
  forceUpdateRequested = false;
  immediateBurstRequested = false;
}

void FloorHeatingSensor::abortBurst() {
  burstInProgress = false;
  conversionStarted = false;
  burstIndex = 0;
  for (int i = 0; i < HEATING_TEMP_BURST_COUNT; i++) {
    burstSamples[i] = NAN;
  }
}

void FloorHeatingSensor::startConversion(unsigned long now) {
  sensors.requestTemperatures();
  conversionStarted = true;
  conversionStartTime = now;
}

void FloorHeatingSensor::startBurst(unsigned long now) {
  if (sensors.getDeviceCount() == 0) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ ERROR: Circle " + String(circleIndex) + " sensor not found");
    }
    failedBurstCount++;
    lastBurstTime = now;
    immediateBurstRequested = false;
    forceUpdateRequested = false;
    if (failedBurstCount >= HEATING_TEMP_MAX_FAILED_BURSTS && !hasError) {
      hasError = true;
      lastTemperature = NAN;
      publishSensorError("Temperature sensor not found");
    }
    return;
  }

  burstInProgress = true;
  burstIndex = 0;
  for (int i = 0; i < HEATING_TEMP_BURST_COUNT; i++) {
    burstSamples[i] = NAN;
  }
  startConversion(now);

  if (DEBUG_SERIAL) {
    Serial.println("🌡️ Circle " + String(circleIndex) + " burst started (" +
                   String(HEATING_TEMP_BURST_COUNT) + " readings)");
  }
}

float FloorHeatingSensor::readRawTemperature() {
  float temp = sensors.getTempCByIndex(0);
  if (temp == -127.0 || isnan(temp)) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ Failed to read DS18B20 for circle " + String(circleIndex));
    }
    return NAN;
  }
  return temp;
}

bool FloorHeatingSensor::isValidTemperature(float temp) const {
  if (isnan(temp) || temp == -127.0) {
    return false;
  }
  return temp >= HEATING_TEMP_MIN_VALID && temp <= HEATING_TEMP_MAX_VALID;
}

float FloorHeatingSensor::computeBurstResult() const {
  float valid[HEATING_TEMP_BURST_COUNT];
  int count = 0;
  for (int i = 0; i < HEATING_TEMP_BURST_COUNT; i++) {
    if (isValidTemperature(burstSamples[i])) {
      valid[count++] = burstSamples[i];
    }
  }

  // Need at least 2 valid samples in the burst
  if (count < 2) {
    return NAN;
  }
  if (count == 2) {
    return (valid[0] + valid[1]) / 2.0f;
  }

  // count == 3: median
  if (valid[0] > valid[1]) {
    float t = valid[0]; valid[0] = valid[1]; valid[1] = t;
  }
  if (valid[1] > valid[2]) {
    float t = valid[1]; valid[1] = valid[2]; valid[2] = t;
  }
  if (valid[0] > valid[1]) {
    float t = valid[0]; valid[0] = valid[1]; valid[1] = t;
  }
  return valid[1];
}

void FloorHeatingSensor::publishSensorError(const char* message) {
  if (mqttManager == nullptr || !mqttManager->isMQTTConnected()) {
    return;
  }
  String errorTopic = "smartcamper/errors/module-3/circle/" + String(circleIndex);
  String errorPayload = "{\"error\":true,\"type\":\"sensor_disconnected\",\"message\":\"";
  errorPayload += message;
  errorPayload += "\",\"timestamp\":";
  errorPayload += String(millis() / 1000);
  errorPayload += "}";
  mqttManager->publishRaw(errorTopic, errorPayload);
}

void FloorHeatingSensor::finishBurst(unsigned long now) {
  burstInProgress = false;
  conversionStarted = false;
  lastBurstTime = now;
  immediateBurstRequested = false;

  bool forcePublish = forceUpdateRequested;
  forceUpdateRequested = false;

  float result = computeBurstResult();

  if (isnan(result)) {
    failedBurstCount++;
    if (DEBUG_SERIAL) {
      Serial.println("❌ Circle " + String(circleIndex) + " burst failed (" +
                     String(failedBurstCount) + "/" + String(HEATING_TEMP_MAX_FAILED_BURSTS) + ")");
    }
    if (failedBurstCount >= HEATING_TEMP_MAX_FAILED_BURSTS && !hasError) {
      hasError = true;
      lastTemperature = NAN;
      publishSensorError("Temperature sensor disconnected");
    }
    return;
  }

  failedBurstCount = 0;
  if (hasError) {
    hasError = false;
    if (DEBUG_SERIAL) {
      Serial.println("✅ Circle " + String(circleIndex) + " sensor recovered: " + String(result, 1) + "°C");
    }
  }

  lastTemperature = result;

  if (DEBUG_SERIAL) {
    Serial.println("✅ Circle " + String(circleIndex) + " burst OK: " + String(result, 1) + "°C (median)");
  }

  if (controller != nullptr && controller->getCircleMode(circleIndex) == CIRCLE_MODE_TEMP_CONTROL) {
    controller->resetLastCheckTime(circleIndex);
  }

  publishIfNeeded(result, forcePublish);
}

void FloorHeatingSensor::publishIfNeeded(float temperature, bool forcePublish) {
  if (mqttManager == nullptr || !mqttManager->isMQTTConnected()) {
    return;
  }
  if (manager == nullptr || isnan(temperature)) {
    return;
  }

  float roundedTemp = round(temperature);
  if (!forcePublish && !isnan(lastPublishedTemperature) && roundedTemp == lastPublishedTemperature) {
    if (DEBUG_MQTT) {
      Serial.println("⏭️ FloorHeatingSensor: Skip publish circle " + String(circleIndex) +
                     " - unchanged " + String(roundedTemp) + "°C");
    }
    return;
  }

  manager->publishCircleStatus(circleIndex, forcePublish);
}

void FloorHeatingSensor::loop() {
  if (controller == nullptr) {
    return;
  }

  CircleMode currentMode = controller->getCircleMode(circleIndex);
  unsigned long now = millis();

  // Circle OFF: stop measuring and clear stale temperature (UI shows null)
  if (currentMode == CIRCLE_MODE_OFF) {
    if (lastKnownMode != CIRCLE_MODE_OFF) {
      clearMeasurementState();
      if (DEBUG_SERIAL) {
        Serial.println("🌡️ Circle " + String(circleIndex) + " OFF - temperature cleared");
      }
    }
    lastKnownMode = CIRCLE_MODE_OFF;
    return;
  }

  // Transition OFF → TEMP_CONTROL: clear stale data, measure immediately
  if (lastKnownMode == CIRCLE_MODE_OFF) {
    lastTemperature = NAN;
    lastPublishedTemperature = NAN;
    failedBurstCount = 0;
    hasError = false;
    abortBurst();
    lastBurstTime = 0;
    immediateBurstRequested = true;
    if (DEBUG_SERIAL) {
      Serial.println("🌡️ Circle " + String(circleIndex) + " ON - requesting fresh burst");
    }
  }
  lastKnownMode = currentMode;

  // MQTT reconnect → refresh status after next burst
  if (mqttManager != nullptr) {
    bool mqttConnected = mqttManager->isMQTTConnected();
    if (mqttConnected && !lastMQTTState) {
      forceUpdateRequested = true;
      immediateBurstRequested = true;
    }
    lastMQTTState = mqttConnected;
  }

  // Global EMI settle after any relay toggle
  if (isGlobalRelaySettling()) {
    if (conversionStarted) {
      conversionStarted = false;
    }
    return;
  }

  if (!burstInProgress) {
    bool intervalDue = (lastBurstTime == 0) || (now - lastBurstTime >= HEATING_TEMP_INTERVAL_MS);
    if (immediateBurstRequested || forceUpdateRequested || intervalDue) {
      startBurst(now);
    }
    return;
  }

  // Burst in progress: wait for conversion, then read next sample
  if (!conversionStarted) {
    startConversion(now);
    return;
  }

  if (now - conversionStartTime < HEATING_TEMP_CONVERSION_MS) {
    return;
  }

  conversionStarted = false;
  burstSamples[burstIndex] = readRawTemperature();
  if (DEBUG_VERBOSE) {
    Serial.println("🌡️ Circle " + String(circleIndex) + " sample " + String(burstIndex) + ": " +
                   (isnan(burstSamples[burstIndex]) ? "NAN" : String(burstSamples[burstIndex], 1)));
  }
  burstIndex++;

  if (burstIndex < HEATING_TEMP_BURST_COUNT) {
    startConversion(now);
    return;
  }

  finishBurst(now);
}

void FloorHeatingSensor::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("📊 Floor Heating Sensor " + String(circleIndex) + " Status:");
    if (isnan(lastTemperature)) {
      Serial.println("  Last Temperature: (none)");
    } else {
      Serial.println("  Last Temperature: " + String(lastTemperature) + "°C");
    }
    Serial.println("  Burst in progress: " + String(burstInProgress ? "Yes" : "No"));
    Serial.println("  Failed bursts: " + String(failedBurstCount) + "/" + String(HEATING_TEMP_MAX_FAILED_BURSTS));
    Serial.println("  Has error: " + String(hasError ? "Yes" : "No"));
  }
}
