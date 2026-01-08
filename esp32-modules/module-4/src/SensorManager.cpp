// Sensor Manager Implementation
// Base coordinator for module-4

#include "SensorManager.h"
#include <Arduino.h>

// Static pointer to current instance
SensorManager* SensorManager::currentInstance = nullptr;

SensorManager::SensorManager(ModuleManager* moduleMgr) 
  : moduleManager(moduleMgr),
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
  
  // Base implementation - can be extended with specific sensors/actuators
  // Note: ModuleManager.loop() is called separately in main loop
}

void SensorManager::handleForceUpdate() {
  // Base implementation - can be extended with specific force update logic
  if (DEBUG_SERIAL) {
    Serial.println("🔄 Force update requested for module-4");
  }
}

// Static MQTT callback method
void SensorManager::handleMQTTMessage(char* topic, byte* payload, unsigned int length) {
  if (currentInstance) {
    currentInstance->commandHandler.handleMQTTMessage(topic, payload, length);
  }
}

void SensorManager::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("📊 Sensor Manager Status:");
    Serial.println("  Module Manager: " + String(moduleManager != nullptr ? "OK" : "NULL"));
    Serial.println("  Module ID: " + String(MODULE_ID));
  }
}

