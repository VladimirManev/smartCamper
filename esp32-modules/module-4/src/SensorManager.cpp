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
    damperManager(moduleMgr),
    tableManager(moduleMgr) {
  
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
  
  // Initialize table manager
  tableManager.begin();
  
  // Set MQTT callback to SensorManager::handleMQTTMessage (handles force_update, damper, and table commands)
  if (moduleManager) {
    moduleManager->getMQTTManager().setCallback(SensorManager::handleMQTTMessage);
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
  
  // Update damper manager (handles servo movement and button processing)
  damperManager.loop();
  
  // Update table manager (handles relay control and button processing)
  tableManager.loop();
  
  // Note: ModuleManager.loop() is called separately in main loop
}

void SensorManager::handleForceUpdate() {
  if (DEBUG_SERIAL) {
    Serial.println("🔄 Force update requested for module-4");
  }
  
  // Force update all dampers (publish status for all)
  damperManager.forceUpdate();
  
  // Force update table (publish current status)
  tableManager.forceUpdate();
}

// Static MQTT callback method (wrapper for MQTTManager)
void SensorManager::handleMQTTMessage(char* topic, byte* payload, unsigned int length) {
  if (currentInstance) {
    currentInstance->processMQTTMessage(topic, payload, length);
  }
}

// Process MQTT message (instance method)
void SensorManager::processMQTTMessage(char* topic, byte* payload, unsigned int length) {
  String topicStr = String(topic);
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  if (DEBUG_SERIAL) {
    Serial.println("📨 Received MQTT message:");
    Serial.println("  Topic: " + topicStr);
    Serial.println("  Message: " + message);
  }
  
  // Check if it's a force_update command (handled by CommandHandler)
  if (topicStr.endsWith("/force_update")) {
    commandHandler.handleMQTTMessage(topic, payload, length);
    return;
  }
  
  // Check if it's a damper command
  // Topic format: smartcamper/commands/module-4/damper/{index}/set_angle
  if (topicStr.indexOf("/damper/") >= 0) {
    // Forward to damper manager
    damperManager.handleMQTTCommand(message);
    return;
  }
  
  // Check if it's a table command
  // Topic format: smartcamper/commands/module-4/table/{action}
  if (topicStr.indexOf("/table") >= 0) {
    // Forward to table manager
    tableManager.handleMQTTCommand(message);
    return;
  }
  
  // Default: forward to command handler
  commandHandler.handleMQTTMessage(topic, payload, length);
}

void SensorManager::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("📊 Sensor Manager Status:");
    Serial.println("  Module Manager: " + String(moduleManager != nullptr ? "OK" : "NULL"));
    Serial.println("  Module ID: " + String(MODULE_ID));
    damperManager.printStatus();
    tableManager.printStatus();
  }
}

