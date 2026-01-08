// Sensor Manager Implementation
// Base coordinator for module-4

#include "SensorManager.h"
#include "Config.h"
#include <Arduino.h>
#include <ArduinoJson.h>

// Static pointer to current instance
SensorManager* SensorManager::currentInstance = nullptr;

SensorManager::SensorManager(ModuleManager* moduleMgr) 
  : moduleManager(moduleMgr),
    commandHandler(moduleMgr ? &moduleMgr->getMQTTManager() : nullptr, this, MODULE_ID),
    damperManager(moduleMgr) {
  
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
  
  // Initialize damper manager
  damperManager.begin();
  
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
  
  // Update damper manager (handles servo movement and button processing)
  damperManager.loop();
  
  // Note: ModuleManager.loop() is called separately in main loop
}

void SensorManager::handleForceUpdate() {
  if (DEBUG_SERIAL) {
    Serial.println("🔄 Force update requested for module-4");
  }
  
  // Force update all dampers (publish status for all)
  damperManager.forceUpdate();
}

// Static MQTT callback method
void SensorManager::handleMQTTMessage(char* topic, byte* payload, unsigned int length) {
  if (!currentInstance) {
    return;
  }
  
  String topicStr = String(topic);
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  // Check if it's a force_update command (handled by CommandHandler)
  if (topicStr.endsWith("/force_update")) {
    currentInstance->commandHandler.handleMQTTMessage(topic, payload, length);
    return;
  }
  
  // Check if it's a damper command
  // Topic format: smartcamper/commands/module-4/damper/...
  if (topicStr.indexOf("/damper/") >= 0 || message.indexOf("\"type\":\"damper\"") >= 0) {
    // Forward to damper manager
    currentInstance->damperManager.handleMQTTCommand(message);
    return;
  }
  
  // Default: forward to command handler
  currentInstance->commandHandler.handleMQTTMessage(topic, payload, length);
}

void SensorManager::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("📊 Sensor Manager Status:");
    Serial.println("  Module Manager: " + String(moduleManager != nullptr ? "OK" : "NULL"));
    Serial.println("  Module ID: " + String(MODULE_ID));
    damperManager.printStatus();
  }
}

