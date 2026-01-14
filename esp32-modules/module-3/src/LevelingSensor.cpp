// Leveling Sensor Implementation
// GY-521 (MPU6050) sensor for camper leveling detection

#include "LevelingSensor.h"
#include <Arduino.h>
#include <ArduinoJson.h>

LevelingSensor::LevelingSensor(MQTTManager* mqtt) 
  : mpu(Wire), mqttManager(mqtt), lastReadTime(0), timeoutExpiresAt(0), initialized(false), isActive(false) {
}

void LevelingSensor::begin() {
  if (DEBUG_SERIAL) {
    Serial.println("📐 Leveling Sensor Starting...");
  }
  
  // Initialize I²C with specific pins for ESP32
  Wire.begin(LEVELING_I2C_SDA, LEVELING_I2C_SCL);
  
  // Initialize MPU6050
  byte status = mpu.begin();
  
  if (status != 0) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ ERROR: Leveling Sensor (MPU6050) initialization failed!");
      Serial.println("   Status code: " + String(status));
      Serial.println("   Check I²C connections: SDA=" + String(LEVELING_I2C_SDA) + ", SCL=" + String(LEVELING_I2C_SCL));
    }
    initialized = false;
    return;
  }
  
  // Calibrate sensor - only calibrate gyro (for drift correction)
  // Do NOT calibrate accelerometer - we need gravity as absolute reference for leveling
  // Accelerometer should measure gravity directly, not relative to current position
  if (DEBUG_SERIAL) {
    Serial.println("   Calibrating MPU6050 gyro only (accelerometer uses gravity reference)...");
  }
  delay(1000);  // Give time for sensor to stabilize
  mpu.calcOffsets(true, false);  // Calibrate gyro only, NOT accelerometer (gravity reference)
  
  initialized = true;
  
  if (DEBUG_SERIAL) {
    Serial.println("✅ Leveling Sensor Ready!");
    Serial.println("   I²C SDA: GPIO " + String(LEVELING_I2C_SDA));
    Serial.println("   I²C SCL: GPIO " + String(LEVELING_I2C_SCL));
    Serial.println("   Read interval: " + String(LEVELING_READ_INTERVAL) + "ms");
  }
}

void LevelingSensor::loop() {
  if (!initialized) {
    return;  // Cannot proceed if initialization failed
  }
  
  unsigned long currentTime = millis();
  
  // Check if timeout expired (stop publishing and measuring)
  if (isActive && timeoutExpiresAt > 0 && currentTime >= timeoutExpiresAt) {
    isActive = false;
    timeoutExpiresAt = 0;
    if (DEBUG_SERIAL) {
      Serial.println("⏸️ Leveling Sensor: Timeout expired - stopped publishing and measuring");
    }
  }
  
  // Only measure and publish if active
  if (isActive) {
    // Update MPU6050 (needed for angle calculation)
    mpu.update();
    
    // Check if interval has passed
    if (currentTime - lastReadTime >= LEVELING_READ_INTERVAL) {
      readAndPublishSensor();
      lastReadTime = currentTime;
    }
  } else {
    // When inactive, still update MPU6050 occasionally to keep it responsive
    // But much less frequently (every 5 seconds)
    static unsigned long lastUpdateTime = 0;
    if (currentTime - lastUpdateTime >= 5000) {
      mpu.update();
      lastUpdateTime = currentTime;
    }
  }
}

void LevelingSensor::readAndPublishSensor() {
  if (!initialized || !isActive) {
    return;
  }
  
  // Get angles from MPU6050 (pitch and roll)
  float pitch = mpu.getAngleX();  // Pitch angle (forward/backward tilt)
  float roll = mpu.getAngleY();   // Roll angle (left/right tilt)
  
  // Round to 0.2° precision for cleaner display
  float roundedPitch = round(pitch / 0.2) * 0.2;
  float roundedRoll = round(roll / 0.2) * 0.2;
  
  // Don't log every measurement to serial (too many messages)
  // Data is published to MQTT and can be viewed in frontend
  
  // Publish to MQTT if connected
  if (mqttManager != nullptr && mqttManager->isMQTTConnected()) {
    StaticJsonDocument<128> doc;
    doc["pitch"] = roundedPitch;
    doc["roll"] = roundedRoll;
    
    String payload;
    serializeJson(doc, payload);
    
    String topic = "smartcamper/sensors/" + String(MODULE_ID) + "/leveling";
    mqttManager->publishRaw(topic, payload);
    
    if (DEBUG_MQTT) {
      Serial.println("📤 Published leveling data: " + topic);
    }
  }
}

void LevelingSensor::start() {
  if (!initialized) {
    if (DEBUG_SERIAL) {
      Serial.println("⚠️ Leveling Sensor: Cannot start - not initialized");
    }
    return;
  }
  
  unsigned long currentTime = millis();
  
  // Reset timeout (1 minute from now)
  timeoutExpiresAt = currentTime + LEVELING_TIMEOUT;
  
  // Activate if not already active
  if (!isActive) {
    isActive = true;
    if (DEBUG_SERIAL) {
      Serial.println("▶️ Leveling Sensor: Started - will publish for 60 seconds");
    }
  } else {
    if (DEBUG_SERIAL) {
      Serial.println("🔄 Leveling Sensor: Timeout reset - continuing for 60 more seconds");
    }
  }
  
  // Log current angles when starting/resetting (useful for debugging)
  if (DEBUG_SERIAL) {
    mpu.update();  // Ensure fresh data
    float pitch = mpu.getAngleX();
    float roll = mpu.getAngleY();
    float roundedPitch = round(pitch / 0.2) * 0.2;
    float roundedRoll = round(roll / 0.2) * 0.2;
    Serial.print("📐 Current angles - Pitch: ");
    Serial.print(roundedPitch, 1);
    Serial.print("° | Roll: ");
    Serial.print(roundedRoll, 1);
    Serial.println("°");
  }
}

void LevelingSensor::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("📊 Leveling Sensor Status:");
    Serial.println("  Initialized: " + String(initialized ? "Yes" : "No"));
    Serial.println("  Active: " + String(isActive ? "Yes" : "No"));
    if (initialized) {
      Serial.println("  I²C SDA: GPIO " + String(LEVELING_I2C_SDA));
      Serial.println("  I²C SCL: GPIO " + String(LEVELING_I2C_SCL));
      Serial.println("  Read interval: " + String(LEVELING_READ_INTERVAL) + "ms");
      if (isActive && timeoutExpiresAt > 0) {
        unsigned long remaining = (timeoutExpiresAt > millis()) ? (timeoutExpiresAt - millis()) / 1000 : 0;
        Serial.println("  Timeout remaining: " + String(remaining) + " seconds");
      }
    }
  }
}
