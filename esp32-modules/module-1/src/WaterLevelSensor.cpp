// Water Level Sensor Implementation
// Specific logic for gray water level detection using conductivity-based electrodes

#include "WaterLevelSensor.h"
#include <Arduino.h>

WaterLevelSensor::WaterLevelSensor(MQTTManager* mqtt) {
  // Validate input parameters
  if (mqtt == nullptr) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ ERROR: WaterLevelSensor: mqttManager cannot be nullptr!");
    }
  }
  
  this->mqttManager = mqtt;
  
  // Initialize GPIO pins array (from bottom to top)
  levelPins[0] = WATER_LEVEL_PIN_1;
  levelPins[1] = WATER_LEVEL_PIN_2;
  levelPins[2] = WATER_LEVEL_PIN_3;
  levelPins[3] = WATER_LEVEL_PIN_4;
  levelPins[4] = WATER_LEVEL_PIN_5;
  levelPins[5] = WATER_LEVEL_PIN_6;
  levelPins[6] = WATER_LEVEL_PIN_7;
  
  // Initialize level percentages array
  levelPercentages[0] = LEVEL_PERCENT_1;
  levelPercentages[1] = LEVEL_PERCENT_2;
  levelPercentages[2] = LEVEL_PERCENT_3;
  levelPercentages[3] = LEVEL_PERCENT_4;
  levelPercentages[4] = LEVEL_PERCENT_5;
  levelPercentages[5] = LEVEL_PERCENT_6;
  levelPercentages[6] = LEVEL_PERCENT_7;
  
  // Initialize timing
  lastSensorRead = 0;
  lastDataSent = 0;
  
  // Initialize measurement data
  measurementIndex = 0;
  measurementCount = 0;
  for (int i = 0; i < 5; i++) {
    levelIndices[i] = -1;  // -1 means empty/0%
  }
  
  // Initialize last published value
  lastPublishedLevel = -1.0;  // -1 means no value published yet
  forceUpdateRequested = false;
  lastMQTTState = false;  // Initialize as disconnected
}

void WaterLevelSensor::begin() {
  // Setup GPIO pins
  setupPins();
  
  if (DEBUG_SERIAL) {
    Serial.println("💧 Water Level Sensor initialized");
    Serial.println("   GPIO pins: " + String(WATER_LEVEL_PIN_1) + ", " + 
                   String(WATER_LEVEL_PIN_2) + ", " + String(WATER_LEVEL_PIN_3) + ", " +
                   String(WATER_LEVEL_PIN_4) + ", " + String(WATER_LEVEL_PIN_5) + ", " +
                   String(WATER_LEVEL_PIN_6) + ", " + String(WATER_LEVEL_PIN_7));
  }
}

void WaterLevelSensor::setupPins() {
  // Set all pins to INPUT (no pull-up) initially
  for (int i = 0; i < NUM_LEVEL_PINS; i++) {
    pinMode(levelPins[i], INPUT);
    digitalWrite(levelPins[i], LOW);
  }
}

void WaterLevelSensor::setPinsLow() {
  // Set all pins to LOW (INPUT mode, no pull-up) to minimize current
  // This prevents corrosion by minimizing current flow through water
  for (int i = 0; i < NUM_LEVEL_PINS; i++) {
    pinMode(levelPins[i], INPUT);
    digitalWrite(levelPins[i], LOW);
  }
}

void WaterLevelSensor::loop() {
  // Validate mqttManager pointer
  if (mqttManager == nullptr) {
    return;  // Cannot proceed without MQTT manager
  }
  
  bool mqttConnected = mqttManager->isMQTTConnected();
  
  // Detect MQTT reconnection (transition from disconnected to connected)
  if (mqttConnected && !lastMQTTState) {
    // MQTT just connected/reconnected - send sensor data immediately
    if (DEBUG_SERIAL) {
      Serial.println("🔄 MQTT reconnected - will send water level data immediately");
    }
    // Force a sensor read and publish on next iteration
    forceUpdateRequested = true;
  }
  
  // Update last known MQTT state
  lastMQTTState = mqttConnected;
  
  // Check if MQTT is connected
  if (!mqttConnected) {
    return;
  }
  
  // Read sensors at intervals OR on force update
  unsigned long currentTime = millis();
  bool isForceUpdate = forceUpdateRequested;
  if (currentTime - lastSensorRead > WATER_LEVEL_READ_INTERVAL || isForceUpdate) {
    lastSensorRead = currentTime;
    
    // Setup all pins to INPUT first (no pull-up)
    setupPins();
    
    // Read water level (returns level index 0-6, or -1 for 0%)
    // This function measures one pin at a time from top to bottom
    int level = readWaterLevel();
    
    // Set all pins to LOW after measurement to minimize current
    setPinsLow();
    
    // Convert level to percentage (handles -1 as 0%)
    float percent = levelToPercent(level);
    
    // Store level index for mode calculation (not percentage - prevents invalid intermediate values)
    levelIndices[measurementIndex] = level;
    measurementIndex = (measurementIndex + 1) % 5;
    if (measurementCount < 5) {
      measurementCount++;
    }
    
    // Calculate mode (most frequent value) every 5 measurements (5 seconds) OR on force update
    if (measurementCount >= 5 && (currentTime - lastDataSent >= WATER_LEVEL_AVERAGE_INTERVAL || isForceUpdate)) {
      // Find mode (most frequent level index)
      int modeLevelIndex = findMode(levelIndices, 5);
      float modePercent = levelToPercent(modeLevelIndex);
      
      // Publish if needed (change detection ≥1% or force update or first publish)
      publishIfNeeded(modePercent, currentTime, isForceUpdate);
      
      // Reset force update flag
      forceUpdateRequested = false;
    } else if (isForceUpdate) {
      // If we don't have 5 measurements yet, just publish current value
      publishIfNeeded(percent, currentTime, true);
      forceUpdateRequested = false;
    }
  }
}

int WaterLevelSensor::readWaterLevel() {
  // Measure one pin at a time from top to bottom
  // This prevents all pins being PULLUP simultaneously which makes water more positive
  // Strategy: Set one pin to PULLUP, measure, if LOW (covered) -> return that level
  //           If HIGH (not covered), set it to INPUT and check next pin
  
  // Start from top (pin 7, index 6, 100%) and go down
  for (int i = NUM_LEVEL_PINS - 1; i >= 0; i--) {
    // Set only this pin to INPUT_PULLUP
    pinMode(levelPins[i], INPUT_PULLUP);
    delay(5);  // Small delay for pull-up to stabilize
    
    // Read this pin
    int pinState = digitalRead(levelPins[i]);
    
    // If pin is LOW, it's covered by water (connected to GND through water)
    if (pinState == LOW) {
      // This is the highest covered pin - return its index
      // Set this pin back to INPUT before returning
      pinMode(levelPins[i], INPUT);
      digitalWrite(levelPins[i], LOW);
      return i;  // Return level index (0-6)
    }
    
    // Pin is HIGH (not covered), set it back to INPUT and check next pin
    pinMode(levelPins[i], INPUT);
    digitalWrite(levelPins[i], LOW);
  }
  
  // No pins covered = 0%
  return -1;  // Returns -1 if no pins covered
}

float WaterLevelSensor::levelToPercent(int level) {
  // level = -1 means 0%
  if (level < 0 || level >= NUM_LEVEL_PINS) {
    return 0.0;
  }
  
  // Return percentage for this level
  return (float)levelPercentages[level];
}

int WaterLevelSensor::findMode(int* values, int count) {
  // Find the most frequent value (mode) in the array
  // If there's a tie, return the higher value (more conservative)
  
  int maxCount = 0;
  int modeValue = values[0];  // Default to first value
  
  // Count frequency of each value
  for (int i = 0; i < count; i++) {
    int currentValue = values[i];
    int currentCount = 0;
    
    // Count how many times this value appears
    for (int j = 0; j < count; j++) {
      if (values[j] == currentValue) {
        currentCount++;
      }
    }
    
    // If this value appears more often, or same count but higher value
    if (currentCount > maxCount || (currentCount == maxCount && currentValue > modeValue)) {
      maxCount = currentCount;
      modeValue = currentValue;
    }
  }
  
  return modeValue;
}

void WaterLevelSensor::publishIfNeeded(float averagePercent, unsigned long currentTime, bool forcePublish) {
  // Validate mqttManager pointer
  if (mqttManager == nullptr) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ ERROR: Cannot publish - mqttManager is nullptr");
    }
    return;
  }
  
  // Validate sensor reading (reasonable range: 0-100%)
  if (averagePercent < 0.0 || averagePercent > 100.0) {
    if (DEBUG_SERIAL) {
      Serial.println("⚠️ WARNING: Water level out of range: " + String(averagePercent) + "%");
    }
    // Clamp to valid range
    averagePercent = (averagePercent < 0.0) ? 0.0 : (averagePercent > 100.0) ? 100.0 : averagePercent;
  }
  
  // If force publish is requested, always publish
  if (forcePublish) {
    mqttManager->publishSensorData("gray-water/level", averagePercent);
    
    if (DEBUG_SERIAL) {
      Serial.println("Published: smartcamper/sensors/gray-water/level = " + String(averagePercent, 1) + "%");
    }
    
    // Save for comparison
    lastPublishedLevel = averagePercent;
    lastDataSent = currentTime;
    return;
  }
  
  // Normal publishing logic - only publish on change (≥1%) or first read
  bool valueChanged = (abs(averagePercent - lastPublishedLevel) >= WATER_LEVEL_THRESHOLD);
  bool firstPublish = (lastPublishedLevel < 0);
  
  // Publish if there's a change OR first read
  if (valueChanged || firstPublish) {
    mqttManager->publishSensorData("gray-water/level", averagePercent);
    
    if (DEBUG_SERIAL) {
      Serial.println("Published: smartcamper/sensors/gray-water/level = " + String(averagePercent, 1) + "%");
    }
    
    // Save for comparison
    lastPublishedLevel = averagePercent;
    lastDataSent = currentTime;
  }
}

void WaterLevelSensor::forceUpdate() {
  forceUpdateRequested = true;
}

void WaterLevelSensor::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("📊 Water Level Sensor Status:");
    Serial.println("  Last Level: " + String(lastPublishedLevel >= 0 ? String(lastPublishedLevel, 1) + "%" : "N/A"));
    Serial.println("  Measurement Count: " + String(measurementCount) + "/5");
    Serial.println("  Last Data Sent: " + String((millis() - lastDataSent) / 1000) + " seconds ago");
    Serial.println("  Force Update Requested: " + String(forceUpdateRequested ? "Yes" : "No"));
  }
}

