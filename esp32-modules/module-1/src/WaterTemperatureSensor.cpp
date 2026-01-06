// Water Temperature Sensor Implementation
// Specific logic for DS18B20 sensor (OneWire)

#include "WaterTemperatureSensor.h"
#include <Arduino.h>

WaterTemperatureSensor::WaterTemperatureSensor(MQTTManager* mqtt) 
  : oneWire(WATER_TEMP_PIN), sensors(&oneWire) {
  // Validate input parameters
  if (mqtt == nullptr) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ ERROR: WaterTemperatureSensor: mqttManager cannot be nullptr!");
    }
  }
  
  this->mqttManager = mqtt;
  this->lastSensorRead = 0;
  this->lastDataSent = 0;
  this->lastTemperature = 0.0;
  this->forceUpdateRequested = false;
  this->lastMQTTState = false;  // Initialize as disconnected
  
  // Initialize async reading state
  this->conversionStarted = false;
  this->conversionStartTime = 0;
  
  // Initialize temperature averaging
  this->temperatureIndex = 0;
  this->temperatureCount = 0;
  this->lastAverageTime = 0;
  for (int i = 0; i < WATER_TEMP_AVERAGE_COUNT; i++) {
    this->temperatureReadings[i] = 0.0;
  }
}

void WaterTemperatureSensor::begin() {
  sensors.begin();
  
  // Set resolution to 12 bits (0.0625°C precision, default)
  // This gives us 0.1°C accuracy which is sufficient
  sensors.setResolution(12);
  
  // CRITICAL: Disable blocking wait for conversion
  // This allows requestTemperatures() to return immediately and conversion happens in background
  // We must wait for conversion to complete before reading (handled in loop with async state machine)
  sensors.setWaitForConversion(false);
  
  if (DEBUG_SERIAL) {
    Serial.println("🌡️ DS18B20 Water Temperature Sensor initialized");
    Serial.println("   GPIO pin: " + String(WATER_TEMP_PIN));
    
    // Count sensors
    int deviceCount = sensors.getDeviceCount();
    Serial.println("   Found " + String(deviceCount) + " DS18B20 device(s)");
    
    if (deviceCount == 0) {
      Serial.println("⚠️ WARNING: No DS18B20 sensors found on pin " + String(WATER_TEMP_PIN));
    }
  }
}

void WaterTemperatureSensor::loop() {
  // Validate mqttManager pointer
  if (mqttManager == nullptr) {
    return;  // Cannot proceed without MQTT manager
  }
  
  bool mqttConnected = mqttManager->isMQTTConnected();
  
  // Detect MQTT reconnection (transition from disconnected to connected)
  if (mqttConnected && !lastMQTTState) {
    // MQTT just connected/reconnected - send sensor data immediately
    if (DEBUG_SERIAL) {
      Serial.println("🔄 MQTT reconnected - will send water temperature data immediately");
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
  
  // Async temperature reading state machine (non-blocking)
  unsigned long currentTime = millis();
  bool isForceUpdate = forceUpdateRequested;
  
  // Check if sensor is available before attempting to read
  int deviceCount = sensors.getDeviceCount();
  if (deviceCount == 0) {
    // No sensor found - reset state and exit
    if (conversionStarted) {
      conversionStarted = false;
    }
    return;
  }
  
  if (!conversionStarted) {
    // Start a new conversion if interval has passed or force update requested
    if (currentTime - lastSensorRead > WATER_TEMP_READ_INTERVAL || isForceUpdate) {
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
        // Store temperature reading for averaging
        temperatureReadings[temperatureIndex] = temperature;
        temperatureIndex = (temperatureIndex + 1) % WATER_TEMP_AVERAGE_COUNT;
        if (temperatureCount < WATER_TEMP_AVERAGE_COUNT) {
          temperatureCount++;
        }
        
        // Calculate average temperature every 5 seconds OR on force update
        if (temperatureCount >= WATER_TEMP_AVERAGE_COUNT && 
            (currentTime - lastAverageTime >= WATER_TEMP_AVERAGE_INTERVAL || isForceUpdate)) {
          float averageTemperature = calculateAverageTemperature();
          publishIfNeeded(averageTemperature, currentTime, isForceUpdate);
          lastAverageTime = currentTime;
          forceUpdateRequested = false;
        } else if (isForceUpdate) {
          // If we don't have enough measurements yet, just publish current value
          publishIfNeeded(temperature, currentTime, true);
          forceUpdateRequested = false;
        }
        
        // Update last temperature even if not publishing
        lastTemperature = temperature;
      } else {
        if (DEBUG_SERIAL) {
          Serial.println("❌ Invalid water temperature reading!");
        }
        forceUpdateRequested = false;
      }
    }
  }
}

float WaterTemperatureSensor::readTemperature() {
  // Read temperature from first sensor (index 0)
  // Conversion should already be complete when this is called
  float temp = sensors.getTempCByIndex(0);
  
  // Check for errors (DallasTemperature returns -127.0 on error)
  if (temp == -127.0 || isnan(temp)) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ Failed to read temperature from DS18B20");
    }
    return NAN;
  }
  
  return temp;
}

float WaterTemperatureSensor::calculateAverageTemperature() {
  float sum = 0.0;
  for (int i = 0; i < WATER_TEMP_AVERAGE_COUNT; i++) {
    sum += temperatureReadings[i];
  }
  return sum / WATER_TEMP_AVERAGE_COUNT;
}

void WaterTemperatureSensor::publishIfNeeded(float temperature, unsigned long currentTime, bool forcePublish) {
  // Validate mqttManager pointer
  if (mqttManager == nullptr) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ ERROR: Cannot publish - mqttManager is nullptr");
    }
    return;
  }
  
  // Validate sensor reading (reasonable range for water temperature)
  // DS18B20 range: -55 to 125°C, but for water we expect 0-50°C
  if (temperature < -10.0 || temperature > 60.0) {
    if (DEBUG_SERIAL) {
      Serial.println("⚠️ WARNING: Water temperature out of expected range: " + String(temperature) + "°C");
    }
    // Still publish but log warning
  }
  
  // Round to 1 decimal place (0.1°C precision)
  temperature = round(temperature * 10) / 10;
  
  // If force publish is requested, always publish
  if (forcePublish) {
    mqttManager->publishSensorData("gray-water-temperature", temperature);
    
    // Save for comparison
    lastTemperature = temperature;
    lastDataSent = currentTime;
    return;
  }
  
  // Normal publishing logic - only publish on change or first read
  bool tempChanged = (abs(temperature - lastTemperature) >= WATER_TEMP_THRESHOLD);
  
  // Publish if there's a change OR first read
  if (tempChanged || lastTemperature == 0.0) {
    mqttManager->publishSensorData("gray-water-temperature", temperature);
    if (DEBUG_SERIAL) {
      Serial.println("Published: smartcamper/sensors/gray-water-temperature = " + String(temperature, 1));
    }
    
    // Save for comparison
    lastTemperature = temperature;
    lastDataSent = currentTime;
  }
}

void WaterTemperatureSensor::forceUpdate() {
  forceUpdateRequested = true;
}

void WaterTemperatureSensor::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("📊 Water Temperature Sensor Status:");
    Serial.println("  Last Temperature: " + String(lastTemperature) + "°C");
    Serial.println("  Last Data Sent: " + String((millis() - lastDataSent) / 1000) + " seconds ago");
    Serial.println("  Force Update Requested: " + String(forceUpdateRequested ? "Yes" : "No"));
    Serial.println("  Measurement Count: " + String(temperatureCount) + "/" + String(WATER_TEMP_AVERAGE_COUNT));
  }
}

