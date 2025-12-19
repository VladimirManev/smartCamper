// Temperature Sensor Manager Implementation
// Specific logic for temperature sensor

#include "Config.h"
#include "SensorManager.h"

// Static pointer to current instance
SensorManager* SensorManager::currentInstance = nullptr;

SensorManager::SensorManager() : dht(25, DHT22), commandHandler(&mqttManager, this, "temperature-sensor") {
  lastSensorRead = 0;
  lastDataSent = 0;              // Initialize last send
  lastStatusLog = 0;             // Initialize last status log
  lastTemperature = 0.0;
  lastHumidity = 0.0;
  forceUpdateRequested = false;
  
  // Set current instance for static methods
  currentInstance = this;
}

void SensorManager::begin() {
  Serial.begin(115200);
  Serial.println("🌡️ Temperature Sensor Module Starting...");
  
  // Initialize DHT sensor
  dht.begin();
  Serial.println("🌡️ AM2301 DHT22 sensor initialized on pin 25");
  
  // Initialize network
  networkManager.begin();
  
  // Initialize MQTT
  mqttManager.begin();
  
  // Setup callback for commands
  mqttManager.setCallback(handleMQTTMessage);
  
  // Initialize Command Handler
  commandHandler.begin();
  
  Serial.println("✅ Temperature Sensor Module Ready!");
}

void SensorManager::loop() {
  // Update network
  networkManager.loop();
  
  // Update MQTT with WiFi status
  bool wifiConnected = networkManager.isWiFiConnected();
  mqttManager.loop(wifiConnected);
  
  // Update Command Handler
  commandHandler.loop();
  
  // Read sensors at intervals OR on force update
  unsigned long currentTime = millis();
  if (currentTime - lastSensorRead > SENSOR_READ_INTERVAL || forceUpdateRequested) {
    lastSensorRead = currentTime;
    
    // Check connection status
    bool wifiOk = networkManager.isWiFiConnected();
    bool mqttOk = mqttManager.isMQTTConnected();
    
    if (wifiOk && mqttOk) {
      // Read real data from AM2301
      float temperature = readTemperature();
      float humidity = readHumidity();
      
      // Round data
      temperature = round(temperature * 10) / 10;  // To 1 decimal place (23.4°C)
      humidity = round(humidity);                  // To whole number (65%)
      
      // Publish data only if valid
      if (!isnan(temperature) && !isnan(humidity)) {
        bool tempChanged = (abs(temperature - lastTemperature) >= TEMP_THRESHOLD);
        bool humidityChanged = (abs(humidity - lastHumidity) >= HUMIDITY_THRESHOLD);
        bool heartbeatNeeded = (currentTime - lastDataSent > HEARTBEAT_INTERVAL);
        
        // Publish if there's a change OR heartbeat needed OR first read
        if (tempChanged || humidityChanged || heartbeatNeeded || lastTemperature == 0.0) {
          // Publish only changed data OR on heartbeat
          if (tempChanged || heartbeatNeeded || lastTemperature == 0.0) {
            mqttManager.publishSensorData("temperature", temperature);
            Serial.println("Published: smartcamper/sensors/temperature = " + String(temperature, 1));
          }
          
          if (humidityChanged || heartbeatNeeded || lastHumidity == 0.0) {
            mqttManager.publishSensorData("humidity", humidity);
            Serial.println("Published: smartcamper/sensors/humidity = " + String((int)humidity));
          }
          
          // Save for comparison
          lastTemperature = temperature;
          lastHumidity = humidity;
          lastDataSent = currentTime;  // Update last send time
        }
        // If no change and heartbeat not needed - don't print anything
        
        // Reset force update flag
        forceUpdateRequested = false;
      } else {
        if (DEBUG_SERIAL) {
        Serial.println("❌ Invalid sensor readings!");
        }
        forceUpdateRequested = false;
      }
    } else {
      // If not connected, log status periodically (every 30 seconds)
      // This helps see what happens during intervals without data
      if (DEBUG_SERIAL && (currentTime - lastStatusLog > 30000)) {
        lastStatusLog = currentTime;
        Serial.println("⚠️ Skipping sensor read - not connected (WiFi: " + String(wifiOk ? "OK" : "FAIL") + ", MQTT: " + String(mqttOk ? "OK" : "FAIL") + ")");
      }
      forceUpdateRequested = false;
    }
  }
}

float SensorManager::readTemperature() {
  // Read temperature from AM2301
  float temp = dht.readTemperature();
  
  if (isnan(temp)) {
    Serial.println("❌ Failed to read temperature from AM2301");
    return NAN;
  }
  
  return temp;
}

float SensorManager::readHumidity() {
  // Read humidity from AM2301
  float humidity = dht.readHumidity();
  
  if (isnan(humidity)) {
    Serial.println("❌ Failed to read humidity from AM2301");
    return NAN;
  }
  
  return humidity;
}

void SensorManager::handleForceUpdate() {
  forceUpdateRequested = true;
  if (DEBUG_SERIAL) {
    Serial.println("🚀 Force update requested - will read sensor on next loop");
  }
}

// Static MQTT callback method
void SensorManager::handleMQTTMessage(char* topic, byte* payload, unsigned int length) {
  if (currentInstance) {
    currentInstance->commandHandler.handleMQTTMessage(topic, payload, length);
  }
}

void SensorManager::printStatus() {
  Serial.println("📊 Temperature Sensor Status:");
  Serial.println("  WiFi: " + String(networkManager.isWiFiConnected() ? "Connected" : "Disconnected"));
  Serial.println("  IP: " + networkManager.getLocalIP());
  mqttManager.printStatus();
  Serial.println("  Last Temperature: " + String(lastTemperature) + "°C");
  Serial.println("  Last Humidity: " + String(lastHumidity) + "%");
}
