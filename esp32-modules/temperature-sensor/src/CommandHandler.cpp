// Command Handler Implementation
// Handle commands from Backend

#include "CommandHandler.h"
#include "SensorManager.h"
#include <Arduino.h>

CommandHandler::CommandHandler(MQTTManager* mqtt, SensorManager* sensor, String moduleType) {
  this->mqttManager = mqtt;
  this->sensorManager = sensor;
  this->moduleType = moduleType;
  this->lastForceUpdate = 0;
}

void CommandHandler::begin() {
  // Subscribe to commands
  String commandTopic = MQTT_TOPIC_COMMANDS + moduleType + "/#";
  mqttManager->subscribeToCommands(moduleType);
  
  if (DEBUG_SERIAL) {
    Serial.println("📨 Command Handler initialized for: " + moduleType);
    Serial.println("📥 Subscribed to: " + commandTopic);
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
  
  // Извикваме force update в SensorManager
  sensorManager->handleForceUpdate();
}

void CommandHandler::printStatus() {
  if (DEBUG_SERIAL) {
    Serial.println("📨 Command Handler Status:");
    Serial.println("  Module Type: " + moduleType);
    Serial.println("  Last Force Update: " + String(lastForceUpdate));
  }
}
