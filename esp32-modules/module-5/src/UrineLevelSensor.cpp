// Urine Level Sensor Implementation
// Toilet urine level — follows module-1 WaterLevelSensor (conductivity electrodes)

#include "UrineLevelSensor.h"
#include <Arduino.h>

UrineLevelSensor::UrineLevelSensor(MQTTManager* mqtt) {
  if (mqtt == nullptr && DEBUG_SERIAL) {
    Serial.println("❌ ERROR: UrineLevelSensor: mqttManager cannot be nullptr!");
  }

  this->mqttManager = mqtt;

  levelPins[0] = URINE_LEVEL_PIN_1;
  levelPins[1] = URINE_LEVEL_PIN_2;

  levelPercentages[0] = URINE_LEVEL_PERCENT_1;
  levelPercentages[1] = URINE_LEVEL_PERCENT_2;

  lastSensorRead = 0;
  lastDataSent = 0;

  measurementIndex = 0;
  measurementCount = 0;
  for (int i = 0; i < URINE_LEVEL_MODE_SAMPLE_COUNT; i++) {
    levelIndices[i] = -1;
  }

  lastPublishedLevel = -1.0;
  forceUpdateRequested = false;
  lastMQTTState = false;
}

void UrineLevelSensor::begin() {
  setupPins();

  if (DEBUG_SERIAL) {
    Serial.println("🚽 Urine Level Sensor initialized");
    Serial.println("   GPIO pins: " + String(URINE_LEVEL_PIN_1) + " (50%), " +
                   String(URINE_LEVEL_PIN_2) + " (100%)");
  }
}

void UrineLevelSensor::setupPins() {
  for (int i = 0; i < NUM_URINE_LEVEL_PINS; i++) {
    pinMode(levelPins[i], INPUT);
    digitalWrite(levelPins[i], LOW);
  }
}

void UrineLevelSensor::setPinsLow() {
  for (int i = 0; i < NUM_URINE_LEVEL_PINS; i++) {
    pinMode(levelPins[i], INPUT);
    digitalWrite(levelPins[i], LOW);
  }
}

void UrineLevelSensor::loop() {
  if (mqttManager == nullptr) {
    return;
  }

  bool mqttConnected = mqttManager->isMQTTConnected();

  if (mqttConnected && !lastMQTTState) {
    if (DEBUG_SERIAL) {
      Serial.println("🔄 MQTT reconnected - will send toilet urine level immediately");
    }
    forceUpdateRequested = true;
  }

  lastMQTTState = mqttConnected;

  if (!mqttConnected) {
    return;
  }

  unsigned long currentTime = millis();
  bool isForceUpdate = forceUpdateRequested;
  if (currentTime - lastSensorRead > URINE_LEVEL_READ_INTERVAL || isForceUpdate) {
    lastSensorRead = currentTime;

    setupPins();
    int level = readWaterLevel();
    setPinsLow();

    float percent = levelToPercent(level);

    levelIndices[measurementIndex] = level;
    measurementIndex = (measurementIndex + 1) % URINE_LEVEL_MODE_SAMPLE_COUNT;
    if (measurementCount < URINE_LEVEL_MODE_SAMPLE_COUNT) {
      measurementCount++;
    }

    if (measurementCount >= URINE_LEVEL_MODE_SAMPLE_COUNT) {
      int modeLevelIndex = findMode(levelIndices, URINE_LEVEL_MODE_SAMPLE_COUNT);
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

int UrineLevelSensor::readWaterLevel() {
  for (int i = NUM_URINE_LEVEL_PINS - 1; i >= 0; i--) {
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

float UrineLevelSensor::levelToPercent(int level) {
  if (level < 0 || level >= NUM_URINE_LEVEL_PINS) {
    return 0.0;
  }
  return (float)levelPercentages[level];
}

int UrineLevelSensor::findMode(int* values, int count) {
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

void UrineLevelSensor::publishIfNeeded(float averagePercent, unsigned long currentTime, bool forcePublish) {
  if (mqttManager == nullptr) {
    return;
  }

  if (averagePercent < 0.0 || averagePercent > 100.0) {
    if (DEBUG_SERIAL) {
      Serial.println("⚠️ WARNING: Urine level out of range: " + String(averagePercent) + "%");
    }
    averagePercent = (averagePercent < 0.0) ? 0.0 : (averagePercent > 100.0) ? 100.0 : averagePercent;
  }

  if (forcePublish) {
    mqttManager->publishSensorData("toilet/urine/level", averagePercent);

    if (DEBUG_SERIAL) {
      Serial.println("Published: smartcamper/sensors/toilet/urine/level = " + String(averagePercent, 1) + "%");
    }

    lastPublishedLevel = averagePercent;
    lastDataSent = currentTime;
    return;
  }

  bool valueChanged = (abs(averagePercent - lastPublishedLevel) >= URINE_LEVEL_THRESHOLD);
  bool firstPublish = (lastPublishedLevel < 0);

  if (valueChanged || firstPublish) {
    mqttManager->publishSensorData("toilet/urine/level", averagePercent);

    if (DEBUG_SERIAL) {
      Serial.println("Published: smartcamper/sensors/toilet/urine/level = " + String(averagePercent, 1) + "%");
    }

    lastPublishedLevel = averagePercent;
    lastDataSent = currentTime;
  }
}

void UrineLevelSensor::forceUpdate() {
  forceUpdateRequested = true;
}

void UrineLevelSensor::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("📊 Urine Level Sensor Status:");
    Serial.println("  Last Level: " + String(lastPublishedLevel >= 0 ? String(lastPublishedLevel, 1) + "%" : "N/A"));
    Serial.println("  Measurement Count: " + String(measurementCount) + "/" + String(URINE_LEVEL_MODE_SAMPLE_COUNT));
  }
}
