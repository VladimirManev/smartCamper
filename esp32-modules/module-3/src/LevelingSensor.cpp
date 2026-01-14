// Leveling Sensor Implementation
// GY-521 (MPU6050) sensor for camper leveling detection

#include "LevelingSensor.h"
#include <Arduino.h>
#include <ArduinoJson.h>

LevelingSensor::LevelingSensor(MQTTManager* mqtt) 
  : mpu(Wire), mqttManager(mqtt), lastReadTime(0), timeoutExpiresAt(0), initialized(false), isActive(false),
    pitchOffset(0.0), rollOffset(0.0), buttonPressed(false), buttonPressStartTime(0), zeroingInProgress(false) {
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
  
  // Initialize Preferences for storing zero offsets
  preferences.begin("leveling", false);  // false = read-write mode
  
  // Load zero offsets from flash
  loadZeroOffsets();
  
  // Initialize BOOT button (GPIO 0) for zeroing
  pinMode(LEVELING_ZERO_BUTTON_PIN, INPUT_PULLUP);
  
  // Initialize LED (GPIO 2) for visual feedback
  pinMode(LEVELING_LED_PIN, OUTPUT);
  digitalWrite(LEVELING_LED_PIN, LOW);
  
  initialized = true;
  
  if (DEBUG_SERIAL) {
    Serial.println("✅ Leveling Sensor Ready!");
    Serial.println("   I²C SDA: GPIO " + String(LEVELING_I2C_SDA));
    Serial.println("   I²C SCL: GPIO " + String(LEVELING_I2C_SCL));
    Serial.println("   Zero button: GPIO " + String(LEVELING_ZERO_BUTTON_PIN) + " (BOOT)");
    Serial.println("   Zero offsets: Pitch=" + String(pitchOffset, 2) + "°, Roll=" + String(rollOffset, 2) + "°");
    Serial.println("   Read interval: " + String(LEVELING_READ_INTERVAL) + "ms");
  }
}

void LevelingSensor::loop() {
  if (!initialized) {
    return;  // Cannot proceed if initialization failed
  }
  
  unsigned long currentTime = millis();
  
  // Handle zero button (BOOT button)
  handleZeroButton();
  
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
  
  // Apply zero offsets (subtract stored offsets to get relative to zero position)
  pitch -= pitchOffset;
  roll -= rollOffset;
  
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
      Serial.println("▶️ Leveling Sensor: Started - will publish for " + String(LEVELING_TIMEOUT / 1000) + " seconds");
    }
  } else {
    if (DEBUG_SERIAL) {
      Serial.println("🔄 Leveling Sensor: Timeout reset - continuing for " + String(LEVELING_TIMEOUT / 1000) + " more seconds");
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

void LevelingSensor::loadZeroOffsets() {
  // Load zero offsets from Preferences (flash memory)
  pitchOffset = preferences.getFloat("pitchOffset", 0.0);
  rollOffset = preferences.getFloat("rollOffset", 0.0);
  
  if (DEBUG_SERIAL) {
    Serial.println("📐 Loaded zero offsets from flash:");
    Serial.println("   Pitch offset: " + String(pitchOffset, 2) + "°");
    Serial.println("   Roll offset: " + String(rollOffset, 2) + "°");
  }
}

void LevelingSensor::saveZeroOffsets(float pitch, float roll) {
  // Save zero offsets to Preferences (flash memory)
  preferences.putFloat("pitchOffset", pitch);
  preferences.putFloat("rollOffset", roll);
  
  // Update in-memory values
  pitchOffset = pitch;
  rollOffset = roll;
  
  if (DEBUG_SERIAL) {
    Serial.println("💾 Saved zero offsets to flash:");
    Serial.println("   Pitch offset: " + String(pitchOffset, 2) + "°");
    Serial.println("   Roll offset: " + String(rollOffset, 2) + "°");
  }
}

void LevelingSensor::handleZeroButton() {
  // Read BOOT button state (GPIO 0, LOW when pressed due to pull-up)
  bool buttonState = digitalRead(LEVELING_ZERO_BUTTON_PIN) == LOW;
  unsigned long currentTime = millis();
  
  if (buttonState && !buttonPressed) {
    // Button just pressed
    buttonPressed = true;
    buttonPressStartTime = currentTime;
    zeroingInProgress = false;
  } else if (buttonState && buttonPressed && !zeroingInProgress) {
    // Button held - check if hold time reached
    unsigned long holdTime = currentTime - buttonPressStartTime;
    if (holdTime >= LEVELING_ZERO_BUTTON_HOLD_TIME) {
      // Long press detected - zero the leveling
      zeroingInProgress = true;
      
      // Get current angles (before applying offsets)
      mpu.update();
      float currentPitch = mpu.getAngleX();
      float currentRoll = mpu.getAngleY();
      
      // Save as zero offsets
      saveZeroOffsets(currentPitch, currentRoll);
      
      // Visual feedback: blink LED 3 times
      blinkLED(3, 100);
      
      if (DEBUG_SERIAL) {
        Serial.println("✅ Leveling zeroed! Current position saved as 0,0");
        Serial.println("   Hold BOOT button for 3 seconds to zero again");
      }
    }
  } else if (!buttonState && buttonPressed) {
    // Button released
    buttonPressed = false;
    zeroingInProgress = false;
  }
}

void LevelingSensor::blinkLED(int times, int duration) {
  // Blink built-in LED (GPIO 2) for visual feedback
  for (int i = 0; i < times; i++) {
    digitalWrite(LEVELING_LED_PIN, HIGH);
    delay(duration);
    digitalWrite(LEVELING_LED_PIN, LOW);
    if (i < times - 1) {
      delay(duration);  // Pause between blinks
    }
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
      Serial.println("  Zero offsets: Pitch=" + String(pitchOffset, 2) + "°, Roll=" + String(rollOffset, 2) + "°");
      Serial.println("  Read interval: " + String(LEVELING_READ_INTERVAL) + "ms");
      if (isActive && timeoutExpiresAt > 0) {
        unsigned long remaining = (timeoutExpiresAt > millis()) ? (timeoutExpiresAt - millis()) / 1000 : 0;
        Serial.println("  Timeout remaining: " + String(remaining) + " seconds");
      }
    }
  }
}
