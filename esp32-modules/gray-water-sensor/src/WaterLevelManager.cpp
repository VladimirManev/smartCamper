// Gray Water Level Manager Implementation
// Specific logic for gray water level sensor

#include "Config.h"
#include "WaterLevelManager.h"
#include <Arduino.h>

// Static pointer to current instance
WaterLevelManager* WaterLevelManager::currentInstance = nullptr;

WaterLevelManager::WaterLevelManager() : commandHandler(&mqttManager, this, "gray-water-sensor") {
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
  lastStatusLog = 0;
  
  // Initialize measurement data
  measurementIndex = 0;
  measurementCount = 0;
  for (int i = 0; i < 5; i++) {
    measurements[i] = 0.0;
  }
  
  // Initialize last published value
  lastPublishedLevel = -1.0;  // -1 means no value published yet
  forceUpdateRequested = false;
  
  // Set current instance for static methods
  currentInstance = this;
}

void WaterLevelManager::begin() {
  Serial.begin(115200);
  Serial.println("💧 Gray Water Level Sensor Module Starting...");
  
  // Setup GPIO pins
  setupPins();
  Serial.println("💧 GPIO pins initialized for water level measurement");
  
  // Initialize network
  networkManager.begin();
  
  // Initialize MQTT
  mqttManager.begin();
  
  // Setup callback for commands
  mqttManager.setCallback(handleMQTTMessage);
  
  // Initialize Command Handler
  commandHandler.begin();
  
  Serial.println("✅ Gray Water Level Sensor Module Ready!");
}

void WaterLevelManager::setupPins() {
  // Configure all level pins as INPUT with pull-up
  for (int i = 0; i < NUM_LEVEL_PINS; i++) {
    pinMode(levelPins[i], INPUT_PULLUP);
  }
}

void WaterLevelManager::setPinsLow() {
  // Set all pins to LOW (INPUT mode, no pull-up) to minimize current
  // This prevents corrosion by minimizing current flow through water
  for (int i = 0; i < NUM_LEVEL_PINS; i++) {
    pinMode(levelPins[i], INPUT);
    digitalWrite(levelPins[i], LOW);
  }
}

void WaterLevelManager::loop() {
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
      // Setup pins for measurement (INPUT_PULLUP)
      setupPins();
      delay(10);  // Small delay for pull-up to stabilize
      
      // Read water level (returns level index 0-6, or -1 for 0%)
      int level = readWaterLevel();
      
      // Set pins to LOW after measurement to minimize current
      setPinsLow();
      
      // Convert level to percentage (handles -1 as 0%)
      float percent = levelToPercent(level);
      
      // Store measurement for averaging
        measurements[measurementIndex] = percent;
        measurementIndex = (measurementIndex + 1) % 5;
        if (measurementCount < 5) {
          measurementCount++;
        }
        
        // Calculate average every 5 measurements (5 seconds)
        if (measurementCount >= 5 && (currentTime - lastDataSent >= AVERAGE_INTERVAL || forceUpdateRequested)) {
          float sum = 0.0;
          for (int i = 0; i < 5; i++) {
            sum += measurements[i];
          }
          float averagePercent = sum / 5.0;
          
          // Round to 1 decimal place
          averagePercent = round(averagePercent * 10) / 10;
          
          // Check if value changed or heartbeat needed or first publish
          bool valueChanged = (abs(averagePercent - lastPublishedLevel) > 0.1);  // 0.1% threshold
          bool heartbeatNeeded = (currentTime - lastDataSent > HEARTBEAT_INTERVAL);
          bool firstPublish = (lastPublishedLevel < 0);
          
          // Publish if changed OR heartbeat OR first publish
          if (valueChanged || heartbeatNeeded || firstPublish) {
            mqttManager.publishSensorData("gray-water/level", averagePercent);
            Serial.println("Published: smartcamper/sensors/gray-water/level = " + String(averagePercent, 1) + "%");
            
            lastPublishedLevel = averagePercent;
            lastDataSent = currentTime;
          }
          
        // Reset force update flag
        forceUpdateRequested = false;
      }
    } else {
      // If not connected, log status periodically (every 30 seconds)
      if (DEBUG_SERIAL && (currentTime - lastStatusLog > 30000)) {
        lastStatusLog = currentTime;
        Serial.println("⚠️ Skipping sensor read - not connected (WiFi: " + String(wifiOk ? "OK" : "FAIL") + ", MQTT: " + String(mqttOk ? "OK" : "FAIL") + ")");
      }
      forceUpdateRequested = false;
    }
  }
}

int WaterLevelManager::readWaterLevel() {
  // Measure from top to bottom (pin 7 to pin 1)
  // Stop at first pin that is covered (LOW = covered, HIGH = not covered)
  // When water touches a bolt, it creates connection to GND, so pin reads LOW
  
  for (int i = NUM_LEVEL_PINS - 1; i >= 0; i--) {
    // Read pin state (LOW = covered by water, HIGH = not covered)
    int pinState = digitalRead(levelPins[i]);
    
    if (pinState == LOW) {
      // This pin is covered, return its level index
      // Level index 0 = pin 1 (15%), level index 6 = pin 7 (100%)
      return i;
    }
  }
  
  // No pins covered = 0% (return -1, will be converted to 0% in levelToPercent)
  return -1;
}

float WaterLevelManager::levelToPercent(int level) {
  // level = -1 means 0%
  if (level < 0 || level >= NUM_LEVEL_PINS) {
    return 0.0;
  }
  
  // Return percentage for this level
  return (float)levelPercentages[level];
}

void WaterLevelManager::handleForceUpdate() {
  forceUpdateRequested = true;
  if (DEBUG_SERIAL) {
    Serial.println("🚀 Force update requested - will read sensor on next loop");
  }
}

// Static MQTT callback method
void WaterLevelManager::handleMQTTMessage(char* topic, byte* payload, unsigned int length) {
  if (currentInstance) {
    currentInstance->commandHandler.handleMQTTMessage(topic, payload, length);
  }
}

void WaterLevelManager::printStatus() {
  Serial.println("📊 Gray Water Level Sensor Status:");
  Serial.println("  WiFi: " + String(networkManager.isWiFiConnected() ? "Connected" : "Disconnected"));
  Serial.println("  IP: " + networkManager.getLocalIP());
  mqttManager.printStatus();
  Serial.println("  Last Level: " + String(lastPublishedLevel >= 0 ? String(lastPublishedLevel, 1) + "%" : "N/A"));
  Serial.println("  Measurement Count: " + String(measurementCount));
}

