#include "CommandHandler.h"
#include "AlarmManager.h"
#include <ArduinoJson.h>

CommandHandler* CommandHandler::currentInstance = nullptr;

CommandHandler::CommandHandler(MQTTManager* mqtt, AlarmManager* manager, String moduleId) {
  this->mqttManager = mqtt;
  this->alarmManager = manager;
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

void CommandHandler::handleZone1On(const String& message) {
  bool ignorePir = false;
  if (message.length() > 0 && message != "{}") {
    StaticJsonDocument<128> doc;
    if (deserializeJson(doc, message) == DeserializationError::Ok) {
      ignorePir = doc["ignoreInteriorPir"] | false;
    }
  }

  if (alarmManager != nullptr) {
    alarmManager->getAlarmSystem().armZone1(ignorePir);
  }
}

void CommandHandler::handleZone1Off(const String& message) {
  const char* pin = "";
  StaticJsonDocument<128> doc;
  if (deserializeJson(doc, message) == DeserializationError::Ok) {
    if (doc.containsKey("pin")) {
      pin = doc["pin"];
    }
  }

  if (alarmManager != nullptr) {
    alarmManager->getAlarmSystem().disarmZone1WithPin(pin);
  }
}

void CommandHandler::handleZone2On() {
  if (alarmManager != nullptr) {
    alarmManager->getAlarmSystem().armZone2();
  }
}

void CommandHandler::handleZone2Off() {
  if (alarmManager != nullptr) {
    alarmManager->getAlarmSystem().disarmZone2();
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
    forceUpdate();
    return;
  }

  if (topicStr.indexOf("/zone/1/on") >= 0) {
    handleZone1On(message);
    return;
  }
  if (topicStr.indexOf("/zone/1/off") >= 0) {
    handleZone1Off(message);
    return;
  }
  if (topicStr.indexOf("/zone/2/on") >= 0) {
    handleZone2On();
    return;
  }
  if (topicStr.indexOf("/zone/2/off") >= 0) {
    handleZone2Off();
    return;
  }
}

void CommandHandler::forceUpdate() {
  lastForceUpdate = millis();
  if (alarmManager != nullptr) {
    alarmManager->handleForceUpdate();
  } else if (DEBUG_SERIAL) {
    Serial.println("ERROR: Cannot force update - AlarmManager not available");
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
