// Floor Heating Temperature Sensor Implementation
// Specific logic for DS18B20 sensor (OneWire) for each heating circle

#include "FloorHeatingSensor.h"
#include "FloorHeatingController.h"
#include "FloorHeatingManager.h"
#include <Arduino.h>

FloorHeatingSensor::FloorHeatingSensor(MQTTManager* mqtt, uint8_t circleIndex, uint8_t pin) 
  : oneWire(pin), sensors(&oneWire) {
  // Validate input parameters
  if (mqtt == nullptr) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ ERROR: FloorHeatingSensor: mqttManager cannot be nullptr!");
    }
  }
  
  if (circleIndex >= NUM_HEATING_CIRCLES) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ ERROR: FloorHeatingSensor: circleIndex out of range!");
    }
  }
  
  this->mqttManager = mqtt;
  this->circleIndex = circleIndex;
  this->pin = pin;
  this->lastSensorRead = 0;
  this->lastDataSent = 0;
  this->lastTemperature = 0.0;
  this->lastPublishedTemperature = NAN;  // Initialize as NAN to force first publish
  this->forceUpdateRequested = false;
  this->lastMQTTState = false;  // Initialize as disconnected
  
  // Initialize async reading state
  this->conversionStarted = false;
  this->conversionStartTime = 0;
  
  // Initialize async reading state
  this->conversionStarted = false;
  this->conversionStartTime = 0;
  
  // Initialize error handling
  this->failedReadCount = 0;
  this->hasError = false;
  this->controller = nullptr;
  this->manager = nullptr;
  
  // Initialize temperature averaging
  this->temperatureIndex = 0;
  this->temperatureCount = 0;
  this->lastAverageTime = 0;
  for (int i = 0; i < HEATING_TEMP_AVERAGE_COUNT; i++) {
    this->temperatureReadings[i] = 0.0;
  }
}

void FloorHeatingSensor::begin() {
  sensors.begin();
  
  // Set resolution to 12 bits (0.0625°C precision, default)
  // This gives us 0.1°C accuracy which is sufficient
  sensors.setResolution(12);
  
  // CRITICAL: Disable blocking wait for conversion
  // This allows requestTemperatures() to return immediately and conversion happens in background
  // We must wait for conversion to complete before reading (handled in readTemperature with yield)
  sensors.setWaitForConversion(false);
  
  if (DEBUG_SERIAL) {
    Serial.println("🌡️ DS18B20 Floor Heating Sensor " + String(circleIndex) + " initialized");
    Serial.println("   GPIO pin: " + String(pin));
    
    // Count sensors
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

void FloorHeatingSensor::loop() {
  // Validate mqttManager pointer
  if (mqttManager == nullptr) {
    return;  // Cannot proceed without MQTT manager
  }
  
  // Track if circle was just turned on (to start reading immediately)
  static CircleMode lastKnownMode = CIRCLE_MODE_OFF;
  static bool circleJustTurnedOnFlag = false;
  bool circleJustTurnedOn = false;
  
  if (controller != nullptr) {
    CircleMode currentMode = controller->getCircleMode(circleIndex);
    
    // Check if circle just turned on (transition from OFF to TEMP_CONTROL)
    if (lastKnownMode == CIRCLE_MODE_OFF && currentMode == CIRCLE_MODE_TEMP_CONTROL) {
      // Circle just turned on - set flag for first measurement only
      circleJustTurnedOnFlag = true;
      circleJustTurnedOn = true;
      // Reset lastSensorRead to force immediate start
      lastSensorRead = 0;
    } else {
      // Circle is not just turned on - reset flag
      circleJustTurnedOn = false;
      circleJustTurnedOnFlag = false;  // Always reset flag if not just turned on
    }
    lastKnownMode = currentMode;
    
    if (currentMode == CIRCLE_MODE_OFF) {
      // Circle is OFF - don't measure temperature
      // Reset error state when circle is turned off
      if (hasError) {
        hasError = false;
        failedReadCount = 0;
      }
      // Reset conversion state when turning off
      if (conversionStarted) {
        conversionStarted = false;
      }
      return;
    }
  }
  
  bool mqttConnected = mqttManager->isMQTTConnected();
  
  // Detect MQTT reconnection (transition from disconnected to connected)
  if (mqttConnected && !lastMQTTState) {
    // MQTT just connected/reconnected - send sensor data immediately
    if (DEBUG_SERIAL) {
      Serial.println("🔄 MQTT reconnected - will send floor heating temperature data immediately for circle " + String(circleIndex));
    }
    // Force a sensor read and publish on next iteration
    // But only if we're not already in the middle of a conversion
    if (!conversionStarted) {
      forceUpdateRequested = true;
    }
  }
  
  // Update last known MQTT state
  lastMQTTState = mqttConnected;
  
  // Async temperature reading state machine (non-blocking)
  unsigned long currentTime = millis();
  bool isForceUpdate = forceUpdateRequested;
  
  // Check if sensor is available before attempting to read
  int deviceCount = sensors.getDeviceCount();
  if (deviceCount == 0) {
    // No sensor found - report error but don't exit early
    // This allows error handling to work properly
    if (conversionStarted) {
      conversionStarted = false;  // Reset state
    }
    // Report error if we haven't already
    if (!hasError && failedReadCount < 3) {
      failedReadCount = 3;  // Trigger error immediately
      hasError = true;
      if (DEBUG_SERIAL) {
        Serial.println("❌ ERROR: Circle " + String(circleIndex) + " sensor not found");
      }
      // Publish error
      if (mqttManager != nullptr && mqttManager->isMQTTConnected()) {
        String errorTopic = "smartcamper/errors/module-3/circle/" + String(circleIndex);
        String errorPayload = "{\"error\":true,\"type\":\"sensor_disconnected\",\"message\":\"Temperature sensor not found\",\"timestamp\":" + String(millis() / 1000) + "}";
        mqttManager->publishRaw(errorTopic, errorPayload);
      }
    }
    return;  // Exit early if no sensor
  }
  
  if (!conversionStarted) {
    // Start a new conversion ONLY if interval has passed (5 seconds)
    // This ensures we don't start new conversion immediately after previous one
    bool intervalPassed = (currentTime - lastSensorRead >= HEATING_TEMP_READ_INTERVAL);
    
    // Allow immediate start ONLY for first measurement (when lastSensorRead is 0)
    // This happens when circle just turned on or on first boot
    // NOTE: Do NOT use isForceUpdate or circleJustTurnedOn here - they cause continuous conversions
    bool isFirstMeasurement = (lastSensorRead == 0);
    
    if (intervalPassed || isFirstMeasurement) {
      sensors.requestTemperatures();  // Start conversion (non-blocking)
      conversionStarted = true;
      conversionStartTime = currentTime;
    }
  } else {
    // Check if conversion is complete (non-blocking check)
    // For 12-bit resolution, conversion takes ~750ms
    // Wait at least 800ms to ensure conversion is complete (safety margin)
    unsigned long elapsed = currentTime - conversionStartTime;
    if (elapsed >= 800) {  // Minimum time for 12-bit conversion + safety margin
      // Conversion should be complete - read temperature
      lastSensorRead = currentTime;
      conversionStarted = false;
      
      // Read data from sensor
      float temperature = readTemperature();
      
      // Process if valid
      if (!isnan(temperature) && temperature != -127.0) {  // -127.0 is DallasTemperature error value
      // Valid reading - reset error counter
      if (failedReadCount > 0) {
        failedReadCount = 0;
      }
      
      // If we had an error and now have valid reading, recover
      if (hasError) {
        hasError = false;
        failedReadCount = 0;  // Reset error counter
        if (DEBUG_SERIAL) {
          Serial.println("✅ Circle " + String(circleIndex) + " sensor recovered - temperature: " + String(temperature, 1) + "°C");
        }
        // Don't publish here - wait for averaging
      }
      
      // Store temperature reading for averaging
      temperatureReadings[temperatureIndex] = temperature;
      temperatureIndex = (temperatureIndex + 1) % HEATING_TEMP_AVERAGE_COUNT;
      if (temperatureCount < HEATING_TEMP_AVERAGE_COUNT) {
        temperatureCount++;
      }
      
      // Calculate average temperature on every 6th measurement (30 seconds)
      // ONLY publish when we have averaged temperature
      if (temperatureCount >= HEATING_TEMP_AVERAGE_COUNT && 
          (currentTime - lastAverageTime >= HEATING_TEMP_AVERAGE_INTERVAL || isForceUpdate)) {
        float averageTemperature = calculateAverageTemperature();
        
        // CRITICAL: Update last temperature with averaged value for local control
        // This ensures the controller uses the averaged temperature for automatic control
        lastTemperature = averageTemperature;
        
        // If circle is in TEMP_CONTROL mode, immediately trigger controller update
        // This ensures relay turns on/off based on averaged temperature
        if (controller != nullptr) {
          CircleMode currentMode = controller->getCircleMode(circleIndex);
          if (currentMode == CIRCLE_MODE_TEMP_CONTROL) {
            controller->resetLastCheckTime(circleIndex);
          }
        }
        
        // ONLY publish when we have averaged temperature (every 6th measurement or 30 seconds)
        // publishIfNeeded will check if temperature has changed and publish only if different
        publishIfNeeded(averageTemperature, currentTime, isForceUpdate);
        lastAverageTime = currentTime;
        forceUpdateRequested = false;
      } else {
        // Don't have enough measurements yet - store temperature for control but don't publish
        // Use current temperature for control (controller needs it immediately)
        lastTemperature = temperature;
        
        // If circle just turned on, trigger controller update immediately
        if (circleJustTurnedOn && circleJustTurnedOnFlag) {
          circleJustTurnedOnFlag = false;
          if (controller != nullptr) {
            CircleMode currentMode = controller->getCircleMode(circleIndex);
            if (currentMode == CIRCLE_MODE_TEMP_CONTROL) {
              controller->resetLastCheckTime(circleIndex);
            }
          }
        }
        
        // DO NOT publish here - wait for averaging (every 6th measurement or 30 seconds)
      }
      } else {
        // Invalid reading - increment error counter
        failedReadCount++;
        if (DEBUG_SERIAL) {
          Serial.println("❌ Invalid floor heating temperature reading for circle " + String(circleIndex) + "! (Failed: " + String(failedReadCount) + "/3)");
        }
        
        // If 3 consecutive failures, trigger error
        if (failedReadCount >= 3 && !hasError) {
          hasError = true;
          if (DEBUG_SERIAL) {
            Serial.println("❌ ERROR: Circle " + String(circleIndex) + " sensor disconnected (3 failed readings)");
          }
          // Publish error and disable circle (via manager callback)
          if (mqttManager != nullptr && mqttManager->isMQTTConnected()) {
            String errorTopic = "smartcamper/errors/module-3/circle/" + String(circleIndex);
            String errorPayload = "{\"error\":true,\"type\":\"sensor_disconnected\",\"message\":\"Temperature sensor disconnected\",\"timestamp\":" + String(millis() / 1000) + "}";
            mqttManager->publishRaw(errorTopic, errorPayload);
          }
          // Disable circle (set to OFF mode) - need manager reference for this
          // Will be handled in FloorHeatingManager
        }
        
        forceUpdateRequested = false;
      }
    }
  }
}

float FloorHeatingSensor::readTemperature() {
  // Read temperature from first sensor (index 0)
  // Conversion should already be complete when this is called
  float temp = sensors.getTempCByIndex(0);
  
  // Check for errors (DallasTemperature returns -127.0 on error)
  if (temp == -127.0 || isnan(temp)) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ Failed to read temperature from DS18B20 for circle " + String(circleIndex));
    }
    return NAN;
  }
  
  return temp;
}

float FloorHeatingSensor::calculateAverageTemperature() {
  float sum = 0.0;
  for (int i = 0; i < HEATING_TEMP_AVERAGE_COUNT; i++) {
    sum += temperatureReadings[i];
  }
  return sum / HEATING_TEMP_AVERAGE_COUNT;
}

void FloorHeatingSensor::publishIfNeeded(float temperature, unsigned long currentTime, bool forcePublish) {
  // Validate mqttManager pointer
  if (mqttManager == nullptr) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ ERROR: Cannot publish - mqttManager is nullptr");
    }
    return;
  }
  
  // Only publish if MQTT is connected
  if (!mqttManager->isMQTTConnected()) {
    return;  // Don't publish if not connected, but keep temperature for local control
  }
  
  // Validate sensor reading (reasonable range for floor heating temperature)
  // DS18B20 range: -55 to 125°C, for floor heating we expect 20 to 50°C
  if (temperature < 15.0 || temperature > 60.0) {
    if (DEBUG_SERIAL) {
      Serial.println("⚠️ WARNING: Floor heating temperature out of expected range: " + String(temperature) + "°C (circle " + String(circleIndex) + ")");
    }
    // Still publish but log warning
  }
  
  // Round to whole number (same as in publishCircleStatus for consistency)
  float roundedTemp = round(temperature);
  
  // Save for comparison and local control (keep original precision for control)
  lastTemperature = temperature;
  if (forcePublish) {
    lastDataSent = currentTime;
  }
  
  // Check if temperature has changed before publishing (only if not forcing)
  if (!forcePublish) {
    float lastPublishedTemp = lastPublishedTemperature;
    if (!isnan(lastPublishedTemp) && roundedTemp == lastPublishedTemp) {
      // Temperature hasn't changed, don't publish
      if (DEBUG_MQTT) {
        Serial.println("⏭️ FloorHeatingSensor: Skipping publish for circle " + String(circleIndex) + " - temperature unchanged: " + String(roundedTemp) + "°C");
      }
      return;
    }
  }
  
  // Publish through manager (which will also check if temperature has changed)
  // Only publish if manager is available and we have a valid temperature
  if (manager != nullptr && roundedTemp > 0 && !isnan(roundedTemp)) {
    if (DEBUG_MQTT) {
      Serial.println("📤 FloorHeatingSensor: Calling publishCircleStatus for circle " + String(circleIndex) + " - temp: " + String(roundedTemp) + "°C, force: " + String(forcePublish));
    }
    manager->publishCircleStatus(circleIndex, forcePublish);
  }
}

void FloorHeatingSensor::forceUpdate() {
  forceUpdateRequested = true;
}

void FloorHeatingSensor::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("📊 Floor Heating Sensor " + String(circleIndex) + " Status:");
    Serial.println("  Last Temperature: " + String(lastTemperature) + "°C");
    Serial.println("  Last Data Sent: " + String((millis() - lastDataSent) / 1000) + " seconds ago");
    Serial.println("  Force Update Requested: " + String(forceUpdateRequested ? "Yes" : "No"));
    Serial.println("  Measurement Count: " + String(temperatureCount) + "/" + String(HEATING_TEMP_AVERAGE_COUNT));
  }
}

