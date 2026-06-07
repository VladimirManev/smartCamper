// Clean Water Level Sensor Implementation
// Conductivity-based electrodes (same approach as module-1 gray water)

#include "CleanWaterLevelSensor.h"
#include <Arduino.h>

CleanWaterLevelSensor::CleanWaterLevelSensor(MQTTManager* mqtt) {
  if (mqtt == nullptr && DEBUG_SERIAL) {
    Serial.println("ERROR: CleanWaterLevelSensor: mqttManager cannot be nullptr!");
  }

  this->mqttManager = mqtt;

  levelPins[0] = CLEAN_WATER_LEVEL_PIN_1;
  levelPins[1] = CLEAN_WATER_LEVEL_PIN_2;
  levelPins[2] = CLEAN_WATER_LEVEL_PIN_3;
  levelPins[3] = CLEAN_WATER_LEVEL_PIN_4;
  levelPins[4] = CLEAN_WATER_LEVEL_PIN_5;
  levelPins[5] = CLEAN_WATER_LEVEL_PIN_6;
  levelPins[6] = CLEAN_WATER_LEVEL_PIN_7;

  levelPercentages[0] = CLEAN_WATER_LEVEL_PERCENT_1;
  levelPercentages[1] = CLEAN_WATER_LEVEL_PERCENT_2;
  levelPercentages[2] = CLEAN_WATER_LEVEL_PERCENT_3;
  levelPercentages[3] = CLEAN_WATER_LEVEL_PERCENT_4;
  levelPercentages[4] = CLEAN_WATER_LEVEL_PERCENT_5;
  levelPercentages[5] = CLEAN_WATER_LEVEL_PERCENT_6;
  levelPercentages[6] = CLEAN_WATER_LEVEL_PERCENT_7;

  lastSensorRead = 0;
  lastDataSent = 0;

  measurementIndex = 0;
  measurementCount = 0;
  for (int i = 0; i < CLEAN_WATER_LEVEL_MODE_SAMPLE_COUNT; i++) {
    levelIndices[i] = -1;
  }

  lastPublishedLevel = -1.0;
  forceUpdateRequested = false;
  lastMQTTState = false;
}

void CleanWaterLevelSensor::begin() {
  setupPins();

  if (DEBUG_SERIAL) {
    Serial.println("Clean Water Level Sensor initialized");
    Serial.println("   GPIO pins: " + String(CLEAN_WATER_LEVEL_PIN_1) + ", " +
                   String(CLEAN_WATER_LEVEL_PIN_2) + ", " + String(CLEAN_WATER_LEVEL_PIN_3) + ", " +
                   String(CLEAN_WATER_LEVEL_PIN_4) + ", " + String(CLEAN_WATER_LEVEL_PIN_5) + ", " +
                   String(CLEAN_WATER_LEVEL_PIN_6) + ", " + String(CLEAN_WATER_LEVEL_PIN_7));
  }
}

void CleanWaterLevelSensor::setupPins() {
  for (int i = 0; i < NUM_CLEAN_WATER_LEVEL_PINS; i++) {
    pinMode(levelPins[i], INPUT);
    digitalWrite(levelPins[i], LOW);
  }
}

void CleanWaterLevelSensor::setPinsLow() {
  for (int i = 0; i < NUM_CLEAN_WATER_LEVEL_PINS; i++) {
    pinMode(levelPins[i], INPUT);
    digitalWrite(levelPins[i], LOW);
  }
}

void CleanWaterLevelSensor::loop() {
  if (mqttManager == nullptr) {
    return;
  }

  bool mqttConnected = mqttManager->isMQTTConnected();

  if (mqttConnected && !lastMQTTState) {
    if (DEBUG_SERIAL) {
      Serial.println("MQTT reconnected - will send clean water level immediately");
    }
    forceUpdateRequested = true;
  }

  lastMQTTState = mqttConnected;

  if (!mqttConnected) {
    return;
  }

  unsigned long currentTime = millis();
  bool isForceUpdate = forceUpdateRequested;
  if (currentTime - lastSensorRead > CLEAN_WATER_LEVEL_READ_INTERVAL || isForceUpdate) {
    lastSensorRead = currentTime;

    setupPins();
    int level = readWaterLevel();
    setPinsLow();

    float percent = levelToPercent(level);

    levelIndices[measurementIndex] = level;
    measurementIndex = (measurementIndex + 1) % CLEAN_WATER_LEVEL_MODE_SAMPLE_COUNT;
    if (measurementCount < CLEAN_WATER_LEVEL_MODE_SAMPLE_COUNT) {
      measurementCount++;
    }

    if (measurementCount >= CLEAN_WATER_LEVEL_MODE_SAMPLE_COUNT) {
      int modeLevelIndex = findMode(levelIndices, CLEAN_WATER_LEVEL_MODE_SAMPLE_COUNT);
      float modePercent = levelToPercent(modeLevelIndex);
      float publishPercent = isForceUpdate ? percent : modePercent;
      publishIfNeeded(publishPercent, currentTime, isForceUpdate);
      forceUpdateRequested = false;
    } else if (isForceUpdate) {
      publishIfNeeded(percent, currentTime, true);
      forceUpdateRequested = false;
    }
  }
}

int CleanWaterLevelSensor::readWaterLevel() {
  // Measure one pin at a time from top to bottom (prevents all pins PULLUP at once)
  for (int i = NUM_CLEAN_WATER_LEVEL_PINS - 1; i >= 0; i--) {
    pinMode(levelPins[i], INPUT_PULLUP);
    delay(5);

    int pinState = digitalRead(levelPins[i]);

    if (pinState == LOW) {
      pinMode(levelPins[i], INPUT);
      digitalWrite(levelPins[i], LOW);
      return i;
    }

    pinMode(levelPins[i], INPUT);
    digitalWrite(levelPins[i], LOW);
  }

  return -1;
}

float CleanWaterLevelSensor::levelToPercent(int level) {
  if (level < 0 || level >= NUM_CLEAN_WATER_LEVEL_PINS) {
    return 0.0;
  }
  return (float)levelPercentages[level];
}

int CleanWaterLevelSensor::findMode(int* values, int count) {
  int maxCount = 0;
  int modeValue = values[0];

  for (int i = 0; i < count; i++) {
    int currentValue = values[i];
    int currentCount = 0;

    for (int j = 0; j < count; j++) {
      if (values[j] == currentValue) {
        currentCount++;
      }
    }

    if (currentCount > maxCount || (currentCount == maxCount && currentValue > modeValue)) {
      maxCount = currentCount;
      modeValue = currentValue;
    }
  }

  return modeValue;
}

void CleanWaterLevelSensor::publishIfNeeded(float averagePercent, unsigned long currentTime, bool forcePublish) {
  if (mqttManager == nullptr) {
    return;
  }

  if (averagePercent < 0.0 || averagePercent > 100.0) {
    if (DEBUG_SERIAL) {
      Serial.println("WARNING: Clean water level out of range: " + String(averagePercent) + "%");
    }
    averagePercent = (averagePercent < 0.0) ? 0.0 : (averagePercent > 100.0) ? 100.0 : averagePercent;
  }

  if (forcePublish) {
    mqttManager->publishSensorData("clean-water/level", averagePercent);

    if (DEBUG_SERIAL) {
      Serial.println("Published: smartcamper/sensors/clean-water/level = " + String(averagePercent, 1) + "%");
    }

    lastPublishedLevel = averagePercent;
    lastDataSent = currentTime;
    return;
  }

  bool valueChanged = (abs(averagePercent - lastPublishedLevel) >= CLEAN_WATER_LEVEL_THRESHOLD);
  bool firstPublish = (lastPublishedLevel < 0);

  if (valueChanged || firstPublish) {
    mqttManager->publishSensorData("clean-water/level", averagePercent);

    if (DEBUG_SERIAL) {
      Serial.println("Published: smartcamper/sensors/clean-water/level = " + String(averagePercent, 1) + "%");
    }

    lastPublishedLevel = averagePercent;
    lastDataSent = currentTime;
  }
}

void CleanWaterLevelSensor::forceUpdate() {
  forceUpdateRequested = true;
}

void CleanWaterLevelSensor::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("Clean Water Level Sensor Status:");
    Serial.println("  Last Level: " + String(lastPublishedLevel >= 0 ? String(lastPublishedLevel, 1) + "%" : "N/A"));
    Serial.println("  Measurement Count: " + String(measurementCount) + "/" + String(CLEAN_WATER_LEVEL_MODE_SAMPLE_COUNT));
    Serial.println("  Last Data Sent: " + String((millis() - lastDataSent) / 1000) + " seconds ago");
  }
}
