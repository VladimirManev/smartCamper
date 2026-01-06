// Outdoor Temperature Sensor Implementation
// Specific logic for DS18B20 sensor (OneWire)

#include "OutdoorTemperatureSensor.h"
#include <Arduino.h>

OutdoorTemperatureSensor::OutdoorTemperatureSensor(MQTTManager* mqtt) 
  : oneWire(OUTDOOR_TEMP_PIN), sensors(&oneWire) {
  // Validate input parameters
  if (mqtt == nullptr) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ ERROR: OutdoorTemperatureSensor: mqttManager cannot be nullptr!");
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
  for (int i = 0; i < OUTDOOR_TEMP_AVERAGE_COUNT; i++) {
    this->temperatureReadings[i] = 0.0;
  }
}

void OutdoorTemperatureSensor::begin() {
  sensors.begin();
  
  // Set resolution to 12 bits (0.0625°C precision, default)
  // This gives us 0.1°C accuracy which is sufficient
  sensors.setResolution(12);
  
  // CRITICAL: Disable blocking wait for conversion
  // This allows requestTemperatures() to return immediately and conversion happens in background
  // We must wait for conversion to complete before reading (handled in loop with async state machine)
  sensors.setWaitForConversion(false);
  
  if (DEBUG_SERIAL) {
    Serial.println("🌡️ DS18B20 Outdoor Temperature Sensor initialized");
    Serial.println("   GPIO pin: " + String(OUTDOOR_TEMP_PIN));
    
    // Count sensors
    int deviceCount = sensors.getDeviceCount();
    Serial.println("   Found " + String(deviceCount) + " DS18B20 device(s)");
    
    if (deviceCount == 0) {
      Serial.println("⚠️ WARNING: No DS18B20 sensors found on pin " + String(OUTDOOR_TEMP_PIN));
    }
  }
}

void OutdoorTemperatureSensor::loop() {
  // Validate mqttManager pointer
  if (mqttManager == nullptr) {
    return;  // Cannot proceed without MQTT manager
  }
  
  bool mqttConnected = mqttManager->isMQTTConnected();
  
  // Detect MQTT reconnection (transition from disconnected to connected)
  if (mqttConnected && !lastMQTTState) {
    // MQTT just connected/reconnected - send sensor data immediately
    if (DEBUG_SERIAL) {
      Serial.println("🔄 MQTT reconnected - will send outdoor temperature data immediately");
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
    if (currentTime - lastSensorRead > OUTDOOR_TEMP_READ_INTERVAL || isForceUpdate) {
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
      temperatureIndex = (temperatureIndex + 1) % OUTDOOR_TEMP_AVERAGE_COUNT;
      if (temperatureCount < OUTDOOR_TEMP_AVERAGE_COUNT) {
        temperatureCount++;
      }
      
      // Calculate average temperature every 5 seconds OR on force update
      if (temperatureCount >= OUTDOOR_TEMP_AVERAGE_COUNT && 
          (currentTime - lastAverageTime >= OUTDOOR_TEMP_AVERAGE_INTERVAL || isForceUpdate)) {
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
        Serial.println("❌ Invalid outdoor temperature reading!");
      }
      forceUpdateRequested = false;
    }
    }
  }
}

float OutdoorTemperatureSensor::readTemperature() {
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

float OutdoorTemperatureSensor::calculateAverageTemperature() {
  float sum = 0.0;
  for (int i = 0; i < OUTDOOR_TEMP_AVERAGE_COUNT; i++) {
    sum += temperatureReadings[i];
  }
  return sum / OUTDOOR_TEMP_AVERAGE_COUNT;
}

void OutdoorTemperatureSensor::publishIfNeeded(float temperature, unsigned long currentTime, bool forcePublish) {
  // Validate mqttManager pointer
  if (mqttManager == nullptr) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ ERROR: Cannot publish - mqttManager is nullptr");
    }
    return;
  }
  
  // Validate sensor reading (reasonable range for outdoor temperature)
  // DS18B20 range: -55 to 125°C, for outdoor we expect -40 to 60°C
  if (temperature < -50.0 || temperature > 70.0) {
    if (DEBUG_SERIAL) {
      Serial.println("⚠️ WARNING: Outdoor temperature out of expected range: " + String(temperature) + "°C");
    }
    // Still publish but log warning
  }
  
  // Round to 1 decimal place (0.1°C precision)
  temperature = round(temperature * 10) / 10;
  
  // If force publish is requested, always publish
  if (forcePublish) {
    mqttManager->publishSensorData("outdoor-temperature", temperature);
    
    // Save for comparison
    lastTemperature = temperature;
    lastDataSent = currentTime;
    return;
  }
  
  // Normal publishing logic - only publish on change or first read
  bool tempChanged = (abs(temperature - lastTemperature) >= OUTDOOR_TEMP_THRESHOLD);
  
  // Publish if there's a change OR first read
  if (tempChanged || lastTemperature == 0.0) {
    mqttManager->publishSensorData("outdoor-temperature", temperature);
    if (DEBUG_SERIAL) {
      Serial.println("Published: smartcamper/sensors/outdoor-temperature = " + String(temperature, 1));
    }
    
    // Save for comparison
    lastTemperature = temperature;
    lastDataSent = currentTime;
  }
}

void OutdoorTemperatureSensor::forceUpdate() {
  forceUpdateRequested = true;
}

void OutdoorTemperatureSensor::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("📊 Outdoor Temperature Sensor Status:");
    Serial.println("  Last Temperature: " + String(lastTemperature) + "°C");
    Serial.println("  Last Data Sent: " + String((millis() - lastDataSent) / 1000) + " seconds ago");
    Serial.println("  Force Update Requested: " + String(forceUpdateRequested ? "Yes" : "No"));
    Serial.println("  Measurement Count: " + String(temperatureCount) + "/" + String(OUTDOOR_TEMP_AVERAGE_COUNT));
  }
}

