// Sensor Manager Implementation
// Coordinator for all sensor classes

#include "SensorManager.h"

// Static pointer to current instance
SensorManager* SensorManager::currentInstance = nullptr;

SensorManager::SensorManager(ModuleManager* moduleMgr) 
  : moduleManager(moduleMgr),
    temperatureHumiditySensor(moduleMgr ? &moduleMgr->getMQTTManager() : nullptr, DHT_PIN, DHT_TYPE),
    commandHandler(moduleMgr ? &moduleMgr->getMQTTManager() : nullptr, this, MODULE_ID) {
  
  // Validate input parameter
  if (moduleMgr == nullptr) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ ERROR: SensorManager: moduleManager cannot be nullptr!");
    }
    // In production, this should trigger error handling
  }
  
  // Set current instance for static methods
  currentInstance = this;
}

void SensorManager::begin() {
  if (DEBUG_SERIAL) {
    Serial.println("📡 Sensor Manager Starting...");
  }
  
  // Initialize temperature/humidity sensor
  temperatureHumiditySensor.begin();
  
  // Command handler will be initialized by ModuleManager
  // (ModuleManager.begin() is called with commandHandler reference)
  
  if (DEBUG_SERIAL) {
    Serial.println("✅ Sensor Manager Ready!");
  }
}

void SensorManager::loop() {
  // Validate moduleManager pointer
  if (moduleManager == nullptr) {
    return;  // Cannot proceed without module manager
  }
  
  // Update all sensors
  // Note: ModuleManager.loop() is called separately in main loop
  temperatureHumiditySensor.loop();
}

void SensorManager::handleForceUpdate() {
  // Force update all sensors
  temperatureHumiditySensor.forceUpdate();
  
  if (DEBUG_SERIAL) {
    Serial.println("🚀 Force update requested for all sensors");
  }
}

// Static MQTT callback method
void SensorManager::handleMQTTMessage(char* topic, byte* payload, unsigned int length) {
  if (currentInstance) {
    currentInstance->commandHandler.handleMQTTMessage(topic, payload, length);
  }
}

void SensorManager::printStatus() const {
  Serial.println("📊 Sensor Manager Status:");
  Serial.println("  Module Manager: " + String(moduleManager != nullptr ? "OK" : "NULL"));
  temperatureHumiditySensor.printStatus();
}
