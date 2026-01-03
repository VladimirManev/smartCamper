// Command Handler Implementation
// Handle commands from Backend

#include "CommandHandler.h"
#include "SensorManager.h"
#include <Arduino.h>

// Static pointer to current instance
CommandHandler* CommandHandler::currentInstance = nullptr;

CommandHandler::CommandHandler(MQTTManager* mqtt, SensorManager* sensor, String moduleId) {
  // Validate input parameters
  if (mqtt == nullptr) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ ERROR: CommandHandler: mqttManager cannot be nullptr!");
    }
  }
  
  if (sensor == nullptr) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ ERROR: CommandHandler: sensorManager cannot be nullptr!");
    }
  }
  
  if (moduleId.length() == 0) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ ERROR: CommandHandler: moduleId cannot be empty!");
    }
  }
  
  this->mqttManager = mqtt;
  this->sensorManager = sensor;
  this->moduleId = moduleId;
  this->lastForceUpdate = 0;
  
  // Set current instance for static methods
  currentInstance = this;
}

void CommandHandler::begin() {
  // Validate mqttManager pointer
  if (mqttManager == nullptr) {
    Serial.println("❌ ERROR: Cannot initialize CommandHandler - mqttManager is nullptr");
    return;
  }
  
  // Subscribe to commands
  String commandTopic = MQTT_TOPIC_COMMANDS + moduleId + "/#";
  bool subscribed = mqttManager->subscribeToCommands(moduleId);
  
  Serial.println("📨 Command Handler initialized for: " + moduleId);
  Serial.println("📥 Subscribed to: " + commandTopic + (subscribed ? " (OK)" : " (FAILED)"));
  Serial.println("📥 Will listen for: " + String(MQTT_TOPIC_COMMANDS) + moduleId + "/force_update");
}

void CommandHandler::loop() {
  // Nothing special for loop
}

void CommandHandler::handleMQTTMessage(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  String topicStr = String(topic);
  
  // Always log received commands for debugging
  Serial.println("📨 Received MQTT command:");
  Serial.println("  Topic: " + topicStr);
  Serial.println("  Message: " + message);
  Serial.println("  Expected module: " + moduleId);
  
  // Check if it's a force_update command
  if (topicStr.endsWith("/force_update")) {
    Serial.println("🔄 Force update command detected!");
    
    // Call force update function
    forceUpdate();
  } else {
    Serial.println("⚠️ Unknown command topic: " + topicStr);
  }
}

void CommandHandler::forceUpdate() {
  lastForceUpdate = millis();
  
  Serial.println("🚀 Force update executed in CommandHandler!");
  
  // Call force update in SensorManager (validate pointer first)
  if (sensorManager != nullptr) {
    Serial.println("📞 Calling SensorManager->handleForceUpdate()...");
    sensorManager->handleForceUpdate();
    Serial.println("✅ SensorManager->handleForceUpdate() called");
  } else {
    Serial.println("❌ ERROR: Cannot force update - sensorManager is nullptr");
  }
}

void CommandHandler::handleMQTTMessageStatic(char* topic, byte* payload, unsigned int length) {
  if (currentInstance) {
    currentInstance->handleMQTTMessage(topic, payload, length);
  }
}

void CommandHandler::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("📨 Command Handler Status:");
    Serial.println("  Module ID: " + moduleId);
    Serial.println("  Last Force Update: " + String((millis() - lastForceUpdate) / 1000) + " seconds ago");
    Serial.println("  MQTT Manager: " + String(mqttManager != nullptr ? "OK" : "NULL"));
    Serial.println("  Sensor Manager: " + String(sensorManager != nullptr ? "OK" : "NULL"));
  }
}
