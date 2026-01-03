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
    if (DEBUG_SERIAL) {
      Serial.println("❌ ERROR: Cannot initialize CommandHandler - mqttManager is nullptr");
    }
    return;
  }
  
  // Subscribe to commands
  String commandTopic = MQTT_TOPIC_COMMANDS + moduleId + "/#";
  bool subscribed = mqttManager->subscribeToCommands(moduleId);
  
  if (DEBUG_SERIAL) {
    Serial.println("📨 Command Handler initialized for: " + moduleId);
    Serial.println("📥 Subscribed to: " + commandTopic + (subscribed ? " (OK)" : " (FAILED)"));
  }
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
  
  if (DEBUG_SERIAL) {
    Serial.println("📨 Received command:");
    Serial.println("  Topic: " + topicStr);
    Serial.println("  Message: " + message);
  }
  
  // Check if it's a force_update command
  if (topicStr.endsWith("/force_update")) {
    if (DEBUG_SERIAL) {
      Serial.println("🔄 Force update requested!");
    }
    
    // Call force update function
    // This will be called from SensorManager
    forceUpdate();
  }
}

void CommandHandler::forceUpdate() {
  lastForceUpdate = millis();
  
  if (DEBUG_SERIAL) {
    Serial.println("🚀 Force update executed!");
  }
  
  // Call force update in SensorManager (validate pointer first)
  if (sensorManager != nullptr) {
    sensorManager->handleForceUpdate();
  } else {
    if (DEBUG_SERIAL) {
      Serial.println("❌ ERROR: Cannot force update - sensorManager is nullptr");
    }
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
