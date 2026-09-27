// Outdoor Temperature Sensor Implementation
// Burst of DS18B20 readings → median every DS18B20_TEMP_INTERVAL_MS

#include "OutdoorTemperatureSensor.h"
#include <Arduino.h>
#include <math.h>

OutdoorTemperatureSensor::OutdoorTemperatureSensor(MQTTManager* mqtt)
  : oneWire(OUTDOOR_TEMP_PIN), sensors(&oneWire) {
  if (mqtt == nullptr && DEBUG_SERIAL) {
    Serial.println("❌ ERROR: OutdoorTemperatureSensor: mqttManager cannot be nullptr!");
  }

  mqttManager = mqtt;
  lastTemperature = NAN;
  lastPublishedTemperature = NAN;
  lastDataSent = 0;
  forceUpdateRequested = false;
  immediateBurstRequested = true;  // First burst ASAP after boot
  lastMQTTState = false;
  burstInProgress = false;
  conversionStarted = false;
  conversionStartTime = 0;
  lastBurstTime = 0;
  burstIndex = 0;
  failedBurstCount = 0;

  for (int i = 0; i < DS18B20_TEMP_BURST_COUNT; i++) {
    burstSamples[i] = NAN;
  }
}

void OutdoorTemperatureSensor::begin() {
  sensors.begin();
  sensors.setResolution(12);
  sensors.setWaitForConversion(false);

  if (DEBUG_SERIAL) {
    Serial.println("🌡️ DS18B20 Outdoor Temperature Sensor initialized");
    Serial.println("   GPIO pin: " + String(OUTDOOR_TEMP_PIN));
    int deviceCount = sensors.getDeviceCount();
    Serial.println("   Found " + String(deviceCount) + " DS18B20 device(s)");
    if (deviceCount == 0) {
      Serial.println("⚠️ WARNING: No DS18B20 sensors found on pin " + String(OUTDOOR_TEMP_PIN));
    }
  }
}

void OutdoorTemperatureSensor::forceUpdate() {
  forceUpdateRequested = true;
  immediateBurstRequested = true;
}

void OutdoorTemperatureSensor::abortBurst() {
  burstInProgress = false;
  conversionStarted = false;
  burstIndex = 0;
  for (int i = 0; i < DS18B20_TEMP_BURST_COUNT; i++) {
    burstSamples[i] = NAN;
  }
}

void OutdoorTemperatureSensor::startConversion(unsigned long now) {
  sensors.requestTemperatures();
  conversionStarted = true;
  conversionStartTime = now;
}

void OutdoorTemperatureSensor::startBurst(unsigned long now) {
  if (sensors.getDeviceCount() == 0) {
    failedBurstCount++;
    lastBurstTime = now;
    immediateBurstRequested = false;
    forceUpdateRequested = false;
    if (DEBUG_SERIAL) {
      Serial.println("❌ Outdoor temp: sensor not found (failed " +
                     String(failedBurstCount) + "/" + String(DS18B20_TEMP_MAX_FAILED_BURSTS) + ")");
    }
    return;
  }

  burstInProgress = true;
  burstIndex = 0;
  for (int i = 0; i < DS18B20_TEMP_BURST_COUNT; i++) {
    burstSamples[i] = NAN;
  }
  startConversion(now);

  if (DEBUG_SERIAL) {
    Serial.println("🌡️ Outdoor temp burst started");
  }
}

float OutdoorTemperatureSensor::readRawTemperature() {
  float temp = sensors.getTempCByIndex(0);
  if (temp == -127.0 || isnan(temp)) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ Failed to read outdoor DS18B20");
    }
    return NAN;
  }
  return temp;
}

bool OutdoorTemperatureSensor::isValidTemperature(float temp) const {
  if (isnan(temp) || temp == -127.0) {
    return false;
  }
  return temp >= OUTDOOR_TEMP_MIN_VALID && temp <= OUTDOOR_TEMP_MAX_VALID;
}

float OutdoorTemperatureSensor::computeBurstResult() const {
  float valid[DS18B20_TEMP_BURST_COUNT];
  int count = 0;
  for (int i = 0; i < DS18B20_TEMP_BURST_COUNT; i++) {
    if (isValidTemperature(burstSamples[i])) {
      valid[count++] = burstSamples[i];
    }
  }

  if (count < 2) {
    return NAN;
  }
  if (count == 2) {
    return (valid[0] + valid[1]) / 2.0f;
  }

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

void OutdoorTemperatureSensor::finishBurst(unsigned long now) {
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
      Serial.println("❌ Outdoor temp burst failed (" +
                     String(failedBurstCount) + "/" + String(DS18B20_TEMP_MAX_FAILED_BURSTS) + ")");
    }
    return;
  }

  failedBurstCount = 0;
  lastTemperature = result;

  if (DEBUG_SERIAL) {
    Serial.println("✅ Outdoor temp burst OK: " + String(result, 1) + "°C (median)");
  }

  publishIfNeeded(result, now, forcePublish);
}

void OutdoorTemperatureSensor::publishIfNeeded(float temperature, unsigned long now, bool forcePublish) {
  if (mqttManager == nullptr || !mqttManager->isMQTTConnected()) {
    return;
  }

  float rounded = round(temperature * 10.0f) / 10.0f;

  if (!forcePublish && !isnan(lastPublishedTemperature) &&
      abs(rounded - lastPublishedTemperature) < OUTDOOR_TEMP_THRESHOLD) {
    return;
  }

  mqttManager->publishSensorData("outdoor-temperature", rounded);
  lastPublishedTemperature = rounded;
  lastDataSent = now;

  if (DEBUG_SERIAL) {
    Serial.println("Published: smartcamper/sensors/outdoor-temperature = " + String(rounded, 1));
  }
}

void OutdoorTemperatureSensor::loop() {
  if (mqttManager == nullptr) {
    return;
  }

  unsigned long now = millis();
  bool mqttConnected = mqttManager->isMQTTConnected();

  if (mqttConnected && !lastMQTTState) {
    if (DEBUG_SERIAL) {
      Serial.println("🔄 MQTT reconnected - outdoor temperature force update");
    }
    forceUpdateRequested = true;
    immediateBurstRequested = true;
  }
  lastMQTTState = mqttConnected;

  // Always measure (offline OK); publish only when connected (in publishIfNeeded)

  if (!burstInProgress) {
    bool intervalDue = (lastBurstTime == 0) || (now - lastBurstTime >= DS18B20_TEMP_INTERVAL_MS);
    if (immediateBurstRequested || forceUpdateRequested || intervalDue) {
      startBurst(now);
    }
    return;
  }

  if (!conversionStarted) {
    startConversion(now);
    return;
  }

  if (now - conversionStartTime < DS18B20_TEMP_CONVERSION_MS) {
    return;
  }

  conversionStarted = false;
  burstSamples[burstIndex] = readRawTemperature();
  burstIndex++;

  if (burstIndex < DS18B20_TEMP_BURST_COUNT) {
    startConversion(now);
    return;
  }

  finishBurst(now);
}

void OutdoorTemperatureSensor::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("📊 Outdoor Temperature Sensor Status:");
    if (isnan(lastTemperature)) {
      Serial.println("  Last Temperature: (none)");
    } else {
      Serial.println("  Last Temperature: " + String(lastTemperature) + "°C");
    }
    Serial.println("  Burst in progress: " + String(burstInProgress ? "Yes" : "No"));
    Serial.println("  Failed bursts: " + String(failedBurstCount) + "/" + String(DS18B20_TEMP_MAX_FAILED_BURSTS));
  }
}
