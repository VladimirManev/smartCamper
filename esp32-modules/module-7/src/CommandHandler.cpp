// Command Handler Implementation

#include "CommandHandler.h"
#include "CleanWaterLevelManager.h"
#include <Arduino.h>

CommandHandler* CommandHandler::currentInstance = nullptr;

CommandHandler::CommandHandler(MQTTManager* mqtt, CleanWaterLevelManager* manager, String moduleId) {
  this->mqttManager = mqtt;
  this->cleanWaterLevelManager = manager;
  this->moduleId = moduleId;
  this->lastForceUpdate = 0;
  this->isSubscribed = false;
  currentInstance = this;
}

void CommandHandler::begin() {
  if (mqttManager == nullptr) {
    if (DEBUG_SERIAL) {
      Serial.println("ERROR: CommandHandler mqttManager is nullptr");
    }
    return;
  }

  if (DEBUG_SERIAL) {
    Serial.println("Command Handler initialized for: " + moduleId);
  }

  isSubscribed = false;
}

void CommandHandler::loop() {
  if (mqttManager != nullptr && mqttManager->isMQTTConnected() && !isSubscribed) {
    String commandTopic = MQTT_TOPIC_COMMANDS + moduleId + "/#";
    bool subscribed = mqttManager->subscribeToCommands(moduleId);

    if (subscribed) {
      isSubscribed = true;
      if (DEBUG_SERIAL) {
        Serial.println("Subscribed to commands: " + commandTopic);
      }
    }
  }

  if (mqttManager != nullptr && !mqttManager->isMQTTConnected() && isSubscribed) {
    isSubscribed = false;
  }
}

void CommandHandler::handleMQTTMessage(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  String topicStr = String(topic);

  if (DEBUG_SERIAL) {
    Serial.println("Received MQTT command:");
    Serial.println("  Topic: " + topicStr);
    Serial.println("  Message: " + message);
  }

  if (topicStr.endsWith("/force_update")) {
    if (DEBUG_SERIAL) {
      Serial.println("Force update command received");
    }
    forceUpdate();
  }
}

void CommandHandler::forceUpdate() {
  lastForceUpdate = millis();

  if (cleanWaterLevelManager != nullptr) {
    cleanWaterLevelManager->handleForceUpdate();
  } else if (DEBUG_SERIAL) {
    Serial.println("ERROR: Cannot force update - CleanWaterLevelManager not available");
  }
}

void CommandHandler::handleMQTTMessageStatic(char* topic, byte* payload, unsigned int length) {
  if (currentInstance) {
    currentInstance->handleMQTTMessage(topic, payload, length);
  }
}

void CommandHandler::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("Command Handler Status:");
    Serial.println("  Module ID: " + moduleId);
    Serial.println("  Last Force Update: " + String((millis() - lastForceUpdate) / 1000) +
                   " seconds ago");
  }
}
