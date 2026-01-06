// Floor Heating Temperature Sensor Implementation
// Specific logic for DS18B20 sensor (OneWire) for each heating circle

#include "FloorHeatingSensor.h"
#include "FloorHeatingController.h"
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
  this->forceUpdateRequested = false;
  this->lastMQTTState = false;  // Initialize as disconnected
  
  // Initialize error handling
  this->failedReadCount = 0;
  this->hasError = false;
  this->controller = nullptr;
  
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

void FloorHeatingSensor::loop() {
  // Validate mqttManager pointer
  if (mqttManager == nullptr) {
    return;  // Cannot proceed without MQTT manager
  }
  
  // Check if circle is in TEMP_CONTROL mode - if not, don't measure
  if (controller != nullptr) {
    CircleMode currentMode = controller->getCircleMode(circleIndex);
    if (currentMode == CIRCLE_MODE_OFF) {
      // Circle is OFF - don't measure temperature
      // Reset error state when circle is turned off
      if (hasError) {
        hasError = false;
        failedReadCount = 0;
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
    forceUpdateRequested = true;
  }
  
  // Update last known MQTT state
  lastMQTTState = mqttConnected;
  
  // Read sensors at intervals OR on force update (works offline too)
  unsigned long currentTime = millis();
  bool isForceUpdate = forceUpdateRequested;
  if (currentTime - lastSensorRead > HEATING_TEMP_READ_INTERVAL || isForceUpdate) {
    lastSensorRead = currentTime;
    
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
        // Publish normal status (frontend will update, but circle stays OFF until user enables it)
        publishIfNeeded(temperature, currentTime, true);
      }
      
      // Store temperature reading for averaging
      temperatureReadings[temperatureIndex] = temperature;
      temperatureIndex = (temperatureIndex + 1) % HEATING_TEMP_AVERAGE_COUNT;
      if (temperatureCount < HEATING_TEMP_AVERAGE_COUNT) {
        temperatureCount++;
      }
      
      // Calculate average temperature every 5 seconds OR on force update
      if (temperatureCount >= HEATING_TEMP_AVERAGE_COUNT && 
          (currentTime - lastAverageTime >= HEATING_TEMP_AVERAGE_INTERVAL || isForceUpdate)) {
        float averageTemperature = calculateAverageTemperature();
        publishIfNeeded(averageTemperature, currentTime, isForceUpdate);
        lastAverageTime = currentTime;
        forceUpdateRequested = false;
      } else if (isForceUpdate) {
        // If we don't have enough measurements yet, just publish current value
        publishIfNeeded(temperature, currentTime, true);
        forceUpdateRequested = false;
      }
      
      // Update last temperature even if not publishing (for local control)
      lastTemperature = temperature;
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

float FloorHeatingSensor::readTemperature() {
  // Request temperature from all sensors
  sensors.requestTemperatures();
  
  // Read temperature from first sensor (index 0)
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
  
  // Round to 1 decimal place (0.1°C precision)
  temperature = round(temperature * 10) / 10;
  
  // Note: We no longer publish temperature-only updates here
  // Temperature is included in the full status published by FloorHeatingManager
  // This keeps the system simpler and ensures mode/relay state is always included
  
  // Save for comparison and local control
  lastTemperature = temperature;
  if (forcePublish) {
    lastDataSent = currentTime;
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

