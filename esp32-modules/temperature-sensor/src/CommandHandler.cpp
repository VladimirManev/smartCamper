// Command Handler Implementation
// Обработка на команди от Backend

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
  // Абонираме се за команди
  String commandTopic = MQTT_TOPIC_COMMANDS + moduleType + "/#";
  mqttManager->subscribeToCommands(moduleType);
  
  if (DEBUG_SERIAL) {
    Serial.println("📨 Command Handler initialized for: " + moduleType);
    Serial.println("📥 Subscribed to: " + commandTopic);
  }
}

void CommandHandler::loop() {
  // Нищо специално за loop
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
  
  // Проверяваме дали е force_update команда
  if (topicStr.endsWith("/force_update")) {
    if (DEBUG_SERIAL) {
      Serial.println("🔄 Force update requested!");
    }
    
    // Извикваме force update функцията
    // Това ще се извика от SensorManager
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
