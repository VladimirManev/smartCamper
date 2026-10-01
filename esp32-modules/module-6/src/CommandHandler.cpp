// Command Handler Implementation

#include "CommandHandler.h"
#include "VictronManager.h"
#include "FridgeManager.h"
#include <Arduino.h>
#include <ArduinoJson.h>

CommandHandler *CommandHandler::currentInstance = nullptr;

CommandHandler::CommandHandler(MQTTManager *mqtt, VictronManager *victronMgr, String moduleId) {
  this->mqttManager = mqtt;
  this->victronManager = victronMgr;
  this->fridgeManager = nullptr;
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

void CommandHandler::handleFridgeCommand(const String &topicStr, const String &message) {
  if (fridgeManager == nullptr) {
    return;
  }

  // .../fridge/mode/eco|max|toggle
  if (topicStr.indexOf("/fridge/mode/") >= 0) {
    if (topicStr.endsWith("/eco")) {
      fridgeManager->setModeEco(true);
    } else if (topicStr.endsWith("/max")) {
      fridgeManager->setModeEco(false);
    } else if (topicStr.endsWith("/toggle")) {
      fridgeManager->setModeToggle();
    }
    return;
  }

  // .../fridge/zone1/set or zone2/set  payload {"temp":N}
  int zone = 0;
  if (topicStr.indexOf("/fridge/zone1/set") >= 0) {
    zone = 1;
  } else if (topicStr.indexOf("/fridge/zone2/set") >= 0) {
    zone = 2;
  }
  if (zone == 0) {
    return;
  }

  StaticJsonDocument<64> doc;
  if (deserializeJson(doc, message)) {
    Serial.println("fridge cmd: bad JSON");
    return;
  }
  if (!doc.containsKey("temp") || !doc["temp"].is<int>()) {
    Serial.println("fridge cmd: missing temp");
    return;
  }
  fridgeManager->setZoneTemp(static_cast<uint8_t>(zone),
                             static_cast<int8_t>(doc["temp"].as<int>()));
}

void CommandHandler::handleMQTTMessage(char *topic, byte *payload, unsigned int length) {
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
    return;
  }

  if (topicStr.indexOf("/fridge/") >= 0) {
    handleFridgeCommand(topicStr, message);
  }
}

void CommandHandler::forceUpdate() {
  lastForceUpdate = millis();

  if (victronManager != nullptr) {
    victronManager->handleForceUpdate();
  } else if (DEBUG_SERIAL) {
    Serial.println("ERROR: Cannot force update - VictronManager not available");
  }

  if (fridgeManager != nullptr) {
    fridgeManager->handleForceUpdate();
  }
}

void CommandHandler::handleMQTTMessageStatic(char *topic, byte *payload, unsigned int length) {
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
