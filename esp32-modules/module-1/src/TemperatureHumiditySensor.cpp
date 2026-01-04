// Temperature and Humidity Sensor Implementation
// Specific logic for DHT22/AM2301 sensor

#include "TemperatureHumiditySensor.h"
#include <Arduino.h>

TemperatureHumiditySensor::TemperatureHumiditySensor(MQTTManager* mqtt, uint8_t pin, uint8_t type) 
  : dht(pin, type) {
  // Validate input parameters
  if (mqtt == nullptr) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ ERROR: TemperatureHumiditySensor: mqttManager cannot be nullptr!");
    }
    // In production, this should trigger error handling or reset
  }
  
  this->mqttManager = mqtt;
  this->lastSensorRead = 0;
  this->lastDataSent = 0;
  this->lastTemperature = 0.0;
  this->lastHumidity = 0.0;
  this->forceUpdateRequested = false;
  this->lastMQTTState = false;  // Initialize as disconnected
  
  // Initialize temperature averaging
  this->temperatureIndex = 0;
  this->temperatureCount = 0;
  this->lastAverageTime = 0;
  for (int i = 0; i < TEMP_AVERAGE_COUNT; i++) {
    this->temperatureReadings[i] = 0.0;
  }
}

void TemperatureHumiditySensor::begin() {
  dht.begin();
  if (DEBUG_SERIAL) {
    Serial.println("🌡️ DHT22/AM2301 sensor initialized");
  }
}

void TemperatureHumiditySensor::loop() {
  // Validate mqttManager pointer
  if (mqttManager == nullptr) {
    return;  // Cannot proceed without MQTT manager
  }
  
  bool mqttConnected = mqttManager->isMQTTConnected();
  
  // Detect MQTT reconnection (transition from disconnected to connected)
  if (mqttConnected && !lastMQTTState) {
    // MQTT just connected/reconnected - send sensor data immediately
    if (DEBUG_SERIAL) {
      Serial.println("🔄 MQTT reconnected - will send sensor data immediately");
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
  if (currentTime - lastSensorRead > SENSOR_READ_INTERVAL || isForceUpdate) {
    lastSensorRead = currentTime;
    
    // Read data from sensor
    float temperature = readTemperature();
    float humidity = readHumidity();
    
    // Publish if valid and needed
    if (!isnan(temperature) && !isnan(humidity)) {
      // Store temperature reading for averaging
      temperatureReadings[temperatureIndex] = temperature;
      temperatureIndex = (temperatureIndex + 1) % TEMP_AVERAGE_COUNT;
      if (temperatureCount < TEMP_AVERAGE_COUNT) {
        temperatureCount++;
      }
      
      // Calculate average temperature every 5 seconds OR on force update
      if (temperatureCount >= TEMP_AVERAGE_COUNT && 
          (currentTime - lastAverageTime >= TEMP_AVERAGE_INTERVAL || isForceUpdate)) {
        float averageTemperature = calculateAverageTemperature();
        publishIfNeeded(averageTemperature, humidity, currentTime, isForceUpdate);
        lastAverageTime = currentTime;
        forceUpdateRequested = false;
      } else if (isForceUpdate) {
        // If we don't have enough measurements yet, just publish current value
        publishIfNeeded(temperature, humidity, currentTime, true);
        forceUpdateRequested = false;
      }
    } else {
      if (DEBUG_SERIAL) {
        Serial.println("❌ Invalid sensor readings!");
      }
      forceUpdateRequested = false;
    }
  }
}

float TemperatureHumiditySensor::readTemperature() {
  float temp = dht.readTemperature();
  
  if (isnan(temp)) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ Failed to read temperature from DHT");
    }
    return NAN;
  }
  
  return temp;
}

float TemperatureHumiditySensor::readHumidity() {
  float humidity = dht.readHumidity();
  
  if (isnan(humidity)) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ Failed to read humidity from DHT");
    }
    return NAN;
  }
  
  return humidity;
}

float TemperatureHumiditySensor::calculateAverageTemperature() {
  float sum = 0.0;
  for (int i = 0; i < TEMP_AVERAGE_COUNT; i++) {
    sum += temperatureReadings[i];
  }
  return sum / TEMP_AVERAGE_COUNT;
}

void TemperatureHumiditySensor::publishIfNeeded(float temperature, float humidity, unsigned long currentTime, bool forcePublish) {
  // Validate mqttManager pointer
  if (mqttManager == nullptr) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ ERROR: Cannot publish - mqttManager is nullptr");
    }
    return;
  }
  
  // Validate sensor readings (reasonable ranges for DHT22)
  // DHT22 range: -40 to 80°C, 0 to 100% humidity
  if (temperature < -40.0 || temperature > 80.0) {
    if (DEBUG_SERIAL) {
      Serial.println("⚠️ WARNING: Temperature out of range: " + String(temperature) + "°C");
    }
    // Still publish but log warning
  }
  
  if (humidity < 0.0 || humidity > 100.0) {
    if (DEBUG_SERIAL) {
      Serial.println("⚠️ WARNING: Humidity out of range: " + String(humidity) + "%");
    }
    // Clamp to valid range
    humidity = (humidity < 0.0) ? 0.0 : (humidity > 100.0) ? 100.0 : humidity;
  }
  
  // Round data
  temperature = round(temperature * 10) / 10;  // To 1 decimal place (23.4°C)
  humidity = round(humidity);                  // To whole number (65%)
  
  // If force publish is requested, always publish both values
  if (forcePublish) {
    mqttManager->publishSensorData("temperature", temperature);
    mqttManager->publishSensorData("humidity", humidity);
    
    // Save for comparison
    lastTemperature = temperature;
    lastHumidity = humidity;
    lastDataSent = currentTime;
    return;
  }
  
  // Normal publishing logic - only publish on change or first read
  bool tempChanged = (abs(temperature - lastTemperature) >= TEMP_THRESHOLD);
  bool humidityChanged = (abs(humidity - lastHumidity) >= HUMIDITY_THRESHOLD);
  
  // Publish if there's a change OR first read
  if (tempChanged || humidityChanged || lastTemperature == 0.0) {
    // Publish only changed data OR on first read
    if (tempChanged || lastTemperature == 0.0) {
      mqttManager->publishSensorData("temperature", temperature);
      if (DEBUG_SERIAL) {
        Serial.println("Published: smartcamper/sensors/temperature = " + String(temperature, 1));
      }
    }
    
    if (humidityChanged || lastHumidity == 0.0) {
      mqttManager->publishSensorData("humidity", humidity);
      if (DEBUG_SERIAL) {
        Serial.println("Published: smartcamper/sensors/humidity = " + String((int)humidity));
      }
    }
    
    // Save for comparison
    lastTemperature = temperature;
    lastHumidity = humidity;
    lastDataSent = currentTime;
  }
}

void TemperatureHumiditySensor::forceUpdate() {
  forceUpdateRequested = true;
}

void TemperatureHumiditySensor::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("📊 Temperature/Humidity Sensor Status:");
    Serial.println("  Last Temperature: " + String(lastTemperature) + "°C");
    Serial.println("  Last Humidity: " + String(lastHumidity) + "%");
    Serial.println("  Last Data Sent: " + String((millis() - lastDataSent) / 1000) + " seconds ago");
    Serial.println("  Force Update Requested: " + String(forceUpdateRequested ? "Yes" : "No"));
  }
}

