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
  
  // Check if MQTT is connected
  if (!mqttManager->isMQTTConnected()) {
    return;
  }
  
  // Read sensors at intervals OR on force update
  unsigned long currentTime = millis();
  if (currentTime - lastSensorRead > SENSOR_READ_INTERVAL || forceUpdateRequested) {
    lastSensorRead = currentTime;
    
    // Read data from sensor
    float temperature = readTemperature();
    float humidity = readHumidity();
    
    // Publish if valid and needed
    if (!isnan(temperature) && !isnan(humidity)) {
      publishIfNeeded(temperature, humidity, currentTime);
      forceUpdateRequested = false;
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

void TemperatureHumiditySensor::publishIfNeeded(float temperature, float humidity, unsigned long currentTime) {
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
  
  // Check for changes
  bool tempChanged = (abs(temperature - lastTemperature) >= TEMP_THRESHOLD);
  bool humidityChanged = (abs(humidity - lastHumidity) >= HUMIDITY_THRESHOLD);
  
  // Publish if there's a change OR first read (no data heartbeat - heartbeat system handles that)
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
  if (DEBUG_SERIAL) {
    Serial.println("🚀 Force update requested for temperature/humidity sensor");
  }
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

