// LED Controller Manager Implementation
// Координира WiFi, MQTT и LED управлението

#include "LEDControllerManager.h"
#include <Arduino.h>
#include <ArduinoJson.h>

// Include main.cpp structures - we need full definition of StripState
// Forward declaration is not enough for accessing members
// We'll access stripStates array which is extern declared in header

// Статичен указател към текущия инстанс
LEDControllerManager* LEDControllerManager::currentInstance = nullptr;

LEDControllerManager::LEDControllerManager() {
  this->lastStatusPublish = 0;
  this->lastHeartbeat = 0;
  this->mqttInitialized = false;
  
  // Задаваме текущия инстанс за статичните методи
  currentInstance = this;
}

void LEDControllerManager::begin() {
  if (DEBUG_SERIAL) {
    Serial.println("💡 LED Controller Manager Starting...");
  }
  
  // Инициализираме мрежата
  networkManager.begin();
  
  // Инициализираме MQTT
  mqttManager.begin();
  
  // Настройваме callback за команди (ще се абонираме след като се свържем)
  mqttManager.setCallback(handleMQTTMessage);
  
  if (DEBUG_SERIAL) {
    Serial.println("✅ LED Controller Manager Ready!");
  }
}

void LEDControllerManager::loop() {
  // Обновяваме мрежата
  networkManager.loop();
  
  // Обновяваме MQTT с WiFi статус
  bool wifiConnected = networkManager.isWiFiConnected();
  mqttManager.loop(wifiConnected);
  
  // Ако MQTT е свързан и все още не сме се абонирали, го правим сега
  if (mqttManager.isMQTTConnected() && !mqttInitialized) {
    mqttManager.subscribeToCommands("led-controller");
    mqttInitialized = true;
    if (DEBUG_SERIAL) {
      Serial.println("📥 Subscribed to MQTT commands for led-controller");
    }
  }
  
  // Ако MQTT се е изключил, ресетираме флага
  if (!mqttManager.isMQTTConnected() && mqttInitialized) {
    mqttInitialized = false;
  }
  
  // Публикуваме heartbeat на интервали (само ако сме свързани)
  if (mqttManager.isMQTTConnected()) {
    unsigned long currentTime = millis();
    if (currentTime - lastHeartbeat > HEARTBEAT_INTERVAL) {
      lastHeartbeat = currentTime;
      publishHeartbeat();
    }
  }
}

void LEDControllerManager::handleMQTTMessage(char* topic, byte* payload, unsigned int length) {
  // Статичен метод - извикваме инстанс метода
  if (currentInstance != nullptr) {
    currentInstance->processMQTTCommand(topic, payload, length);
  }
}

void LEDControllerManager::processMQTTCommand(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  String topicStr = String(topic);
  
  if (DEBUG_SERIAL) {
    Serial.println("📨 Received MQTT command:");
    Serial.println("  Topic: " + topicStr);
    Serial.println("  Message: " + message);
  }
  
  // Check if any button is pressed - ignore MQTT commands if so
  if (isAnyButtonPressed()) {
    if (DEBUG_SERIAL) {
      Serial.println("⚠️ Ignoring MQTT command - button is pressed");
    }
    return;
  }
  
  // Parse topic: smartcamper/commands/led-controller/{command}
  // Expected formats:
  // - smartcamper/commands/led-controller/strip/{index}/on
  // - smartcamper/commands/led-controller/strip/{index}/off
  // - smartcamper/commands/led-controller/strip/{index}/toggle
  // - smartcamper/commands/led-controller/strip/{index}/brightness
  // - smartcamper/commands/led-controller/relay/toggle
  
  String commandPrefix = String(MQTT_TOPIC_COMMANDS) + "led-controller/";
  if (topicStr.startsWith(commandPrefix)) {
    String commandPath = topicStr.substring(commandPrefix.length());
    
    // Handle relay commands
    if (commandPath == "relay/toggle") {
      toggleRelay();
      publishRelayStatus();
      return;
    }
    
    // Handle strip commands
    if (commandPath.startsWith("strip/")) {
      String stripCommand = commandPath.substring(6);  // Remove "strip/"
      
      // Extract strip index (first number)
      int slashIndex = stripCommand.indexOf('/');
      if (slashIndex == -1) {
        if (DEBUG_SERIAL) {
          Serial.println("❌ Invalid strip command format");
        }
        return;
      }
      
      String stripIndexStr = stripCommand.substring(0, slashIndex);
      uint8_t stripIndex = stripIndexStr.toInt();
      
      // Validate strip index
      if (stripIndex >= 4) {  // NUM_STRIPS is 4
        if (DEBUG_SERIAL) {
          Serial.println("❌ Invalid strip index: " + String(stripIndex));
        }
        return;
      }
      
      String action = stripCommand.substring(slashIndex + 1);
      
      if (action == "on") {
        turnOnStrip(stripIndex);
        publishStripStatus(stripIndex);
      } else if (action == "off") {
        turnOffStrip(stripIndex);
        publishStripStatus(stripIndex);
      } else if (action == "toggle") {
        toggleStrip(stripIndex);
        publishStripStatus(stripIndex);
      } else if (action == "brightness") {
        // Parse JSON payload: {"value": 128}
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, message);
        
        if (error) {
          if (DEBUG_SERIAL) {
            Serial.println("❌ Failed to parse JSON: " + String(error.c_str()));
          }
          return;
        }
        
        if (!doc.containsKey("value")) {
          if (DEBUG_SERIAL) {
            Serial.println("❌ Missing 'value' field in JSON");
          }
          return;
        }
        
        uint8_t brightness = doc["value"].as<uint8_t>();
        
        // Clamp brightness
        if (brightness < 1) brightness = 1;
        if (brightness > 255) brightness = 255;
        
        setBrightnessSmooth(stripIndex, brightness);
        // Status will be published when transition completes (in updateDimming)
        // But we'll also publish immediately to update UI
        publishStripStatus(stripIndex);
      } else {
        if (DEBUG_SERIAL) {
          Serial.println("❌ Unknown action: " + action);
        }
      }
    }
  }
}

void LEDControllerManager::publishStatus() {
  // Публикуваме статус на всички ленти
  for (uint8_t i = 0; i < 4; i++) {  // NUM_STRIPS is 4
    publishStripStatus(i);
  }
  
  // Публикуваме статус на релето
  publishRelayStatus();
}

void LEDControllerManager::publishStripStatus(uint8_t stripIndex) {
  if (stripIndex >= 4) return;  // NUM_STRIPS is 4
  
  StripState& state = stripStates[stripIndex];
  
  // Publish state (ON/OFF)
  String stateTopic = "led-controller/strip/" + String(stripIndex) + "/state";
  String stateValue = state.on ? "ON" : "OFF";
  mqttManager.publishSensorData(stateTopic, stateValue);
  
  // Publish brightness
  String brightnessTopic = "led-controller/strip/" + String(stripIndex) + "/brightness";
  mqttManager.publishSensorData(brightnessTopic, (int)state.brightness);
  
  // Reset heartbeat timer when publishing status
  lastHeartbeat = millis();
  
  // Don't log every status publish - too verbose
  // Only log if DEBUG_VERBOSE is enabled
  if (DEBUG_VERBOSE && DEBUG_MQTT) {
    Serial.println("📤 Published strip " + String(stripIndex) + " status: " + stateValue + ", brightness: " + String(state.brightness));
  }
}

void LEDControllerManager::publishRelayStatus() {
  String stateTopic = "led-controller/relay/state";
  String stateValue = relayState ? "ON" : "OFF";
  mqttManager.publishSensorData(stateTopic, stateValue);
  
  // Reset heartbeat timer when publishing status
  lastHeartbeat = millis();
  
  // Don't log every status publish - too verbose
  if (DEBUG_VERBOSE && DEBUG_MQTT) {
    Serial.println("📤 Published relay status: " + stateValue);
  }
}

void LEDControllerManager::publishHeartbeat() {
  // Publish simple heartbeat message - module is alive
  String heartbeatTopic = "led-controller/heartbeat";
  mqttManager.publishSensorData(heartbeatTopic, "alive");
  
  // Don't log heartbeat - too frequent and not useful for debugging
  // if (DEBUG_MQTT) {
  //   Serial.println("💓 Published heartbeat");
  // }
}

bool LEDControllerManager::isWiFiConnected() {
  return networkManager.isWiFiConnected();
}

bool LEDControllerManager::isMQTTConnected() {
  return mqttManager.isMQTTConnected();
}

void LEDControllerManager::printStatus() {
  if (DEBUG_SERIAL) {
    Serial.println("📊 LED Controller Manager Status:");
    Serial.println("  WiFi Connected: " + String(isWiFiConnected() ? "Yes" : "No"));
    Serial.println("  MQTT Connected: " + String(isMQTTConnected() ? "Yes" : "No"));
    if (isWiFiConnected()) {
      Serial.println("  WiFi IP: " + networkManager.getLocalIP());
    }
    mqttManager.printStatus();
  }
}

