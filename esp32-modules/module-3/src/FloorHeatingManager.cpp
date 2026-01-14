// Floor Heating Manager Implementation
// Coordinator for all floor heating functionality

#include "FloorHeatingManager.h"
#include "FloorHeatingButtonHandler.h"  // Include here to avoid circular dependency
#include "LevelingSensor.h"
#include "Config.h"
#include <ArduinoJson.h>

// Static pointer to current instance
FloorHeatingManager* FloorHeatingManager::currentInstance = nullptr;

// Temperature sensor pins
const uint8_t tempPins[NUM_HEATING_CIRCLES] = {
  HEATING_TEMP_PIN_0,
  HEATING_TEMP_PIN_1,
  HEATING_TEMP_PIN_2,
  HEATING_TEMP_PIN_3
};

FloorHeatingManager::FloorHeatingManager(ModuleManager* moduleMgr) 
  : moduleManager(moduleMgr),
    controller(),
    sensors{
      FloorHeatingSensor(moduleMgr ? &moduleMgr->getMQTTManager() : nullptr, 0, tempPins[0]),
      FloorHeatingSensor(moduleMgr ? &moduleMgr->getMQTTManager() : nullptr, 1, tempPins[1]),
      FloorHeatingSensor(moduleMgr ? &moduleMgr->getMQTTManager() : nullptr, 2, tempPins[2]),
      FloorHeatingSensor(moduleMgr ? &moduleMgr->getMQTTManager() : nullptr, 3, tempPins[3])
    },
    buttonHandler(&controller),
    levelingSensor(moduleMgr ? &moduleMgr->getMQTTManager() : nullptr),
    commandHandler(moduleMgr ? &moduleMgr->getMQTTManager() : nullptr, this, MODULE_ID),
    pendingStatusUpdate(false) {
  
  // Validate input parameter
  if (moduleMgr == nullptr) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ ERROR: FloorHeatingManager: moduleManager cannot be nullptr!");
    }
  }
  
  // Set current instance for static methods
  currentInstance = this;
  
  // Link sensors to controller and manager
  for (uint8_t i = 0; i < NUM_HEATING_CIRCLES; i++) {
    controller.setSensor(i, &sensors[i]);
    // Link controller to sensors (for checking circle mode)
    sensors[i].setController(&controller);
    // Link manager to sensors (for publishing status)
    sensors[i].setManager(this);
  }
  
  // Link manager to controller (for status publishing callbacks)
  controller.setManager(this);
}

void FloorHeatingManager::begin() {
  if (DEBUG_SERIAL) {
    Serial.println("🔥 Floor Heating Manager Starting...");
  }
  
  // Initialize controller
  controller.begin();
  
  // Initialize sensors
  for (uint8_t i = 0; i < NUM_HEATING_CIRCLES; i++) {
    sensors[i].begin();
  }
  
  // Initialize button handler
  buttonHandler.setController(&controller);
  buttonHandler.setManager(this);
  buttonHandler.begin();
  
  // Initialize leveling sensor
  levelingSensor.begin();
  
  // Set MQTT callback to FloorHeatingManager::handleMQTTMessage (handles both force_update and heating commands)
  if (moduleManager) {
    moduleManager->getMQTTManager().setCallback(FloorHeatingManager::handleMQTTMessageStatic);
  }
  
  // Command handler will be initialized by ModuleManager
  // (ModuleManager.begin() is called with commandHandler reference)
  
  if (DEBUG_SERIAL) {
    Serial.println("✅ Floor Heating Manager Ready!");
  }
}

void FloorHeatingManager::loop() {
  // Update sensors (read temperature - works offline, only if circle is in TEMP_CONTROL mode)
  for (uint8_t i = 0; i < NUM_HEATING_CIRCLES; i++) {
    sensors[i].loop();
    
    // Check for sensor errors and disable circle if needed
    if (sensors[i].hasSensorError()) {
      // Sensor has error - disable circle (set to OFF mode)
      if (controller.getCircleMode(i) != CIRCLE_MODE_OFF) {
        if (DEBUG_SERIAL) {
          Serial.println("❌ Disabling circle " + String(i) + " due to sensor error");
        }
        controller.setCircleMode(i, CIRCLE_MODE_OFF);
        publishCircleStatus(i, true);  // Force publish because mode changed due to error
      }
    }
    // Note: We don't publish status here when sensor is OK and circle is OFF
    // Status will be published when:
    // 1. Circle mode changes (via button or MQTT command)
    // 2. Relay state changes (via automatic temperature control)
    // 3. Sensor error occurs or recovers (handled above)
  }
  
  // Update controller (automatic temperature control - works offline)
  controller.loop();
  
  // Update button handler (toggle mode - works offline)
  buttonHandler.loop();
  
  // Update leveling sensor (read and log angles)
  levelingSensor.loop();
  
  // Process pending status update (deferred from MQTT callback)
  // This prevents blocking PubSubClient which can cause publish failures
  if (pendingStatusUpdate) {
    pendingStatusUpdate = false;
    publishFullStatus();
  }
}

void FloorHeatingManager::handleForceUpdate() {
  // Set flag to publish status in main loop (don't publish during MQTT callback)
  // This prevents blocking PubSubClient which can cause publish failures
  pendingStatusUpdate = true;
  
  // Also force sensor updates
  for (uint8_t i = 0; i < NUM_HEATING_CIRCLES; i++) {
    sensors[i].forceUpdate();
  }
}

// Static MQTT callback method (wrapper for MQTTManager)
void FloorHeatingManager::handleMQTTMessageStatic(char* topic, byte* payload, unsigned int length) {
  if (currentInstance) {
    currentInstance->processMQTTMessage(topic, payload, length);
  }
}

// Process MQTT message (instance method)
void FloorHeatingManager::processMQTTMessage(char* topic, byte* payload, unsigned int length) {
  String topicStr = String(topic);
  
  if (DEBUG_SERIAL) {
    Serial.println("📨 FloorHeatingManager received MQTT message:");
    Serial.println("  Topic: " + topicStr);
  }
  
  // First try force_update command (handled by CommandHandler)
  if (topicStr.endsWith("/force_update")) {
    commandHandler.handleMQTTMessage(topic, payload, length);
    return;
  }
  
  // Handle floor heating-specific commands
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  // Parse topic: smartcamper/commands/module-3/{command}
  String commandPrefix = String(MQTT_TOPIC_COMMANDS) + MODULE_ID + "/";
  
  if (!topicStr.startsWith(commandPrefix)) {
    if (DEBUG_SERIAL) {
      Serial.println("  ⚠️ Topic doesn't start with command prefix, ignoring");
    }
    return;  // Not our command
  }
  
  String commandPath = topicStr.substring(commandPrefix.length());
  
  if (DEBUG_SERIAL) {
    Serial.println("  Command path: " + commandPath);
  }
  
    // Handle leveling commands: leveling/start
    if (commandPath == "leveling/start") {
      handleLevelingStart();
      return;
    }
    
    // Handle circle commands: circle/{index}/{action}
    if (commandPath.startsWith("circle/")) {
    String circleCommand = commandPath.substring(7);  // Remove "circle/"
    
    // Extract circle index (first number)
    int slashIndex = circleCommand.indexOf('/');
    if (slashIndex == -1) {
      if (DEBUG_SERIAL) {
        Serial.println("❌ Invalid circle command format");
      }
      return;
    }
    
    String circleIndexStr = circleCommand.substring(0, slashIndex);
    uint8_t circleIndex = circleIndexStr.toInt();
    
    // Validate circle index
    if (circleIndex >= NUM_HEATING_CIRCLES) {
      if (DEBUG_SERIAL) {
        Serial.println("❌ Invalid circle index: " + String(circleIndex));
      }
      return;
    }
    
    String action = circleCommand.substring(slashIndex + 1);
    
    if (DEBUG_SERIAL) {
      Serial.println("🔥 Processing circle command:");
      Serial.println("  Circle index: " + String(circleIndex));
      Serial.println("  Action: " + action);
    }
    
    handleCircleCommand(circleIndex, action, message);
  } else {
    if (DEBUG_SERIAL) {
      Serial.println("  ⚠️ Unknown command path: " + commandPath);
    }
  }
}

void FloorHeatingManager::handleCircleCommand(uint8_t circleIndex, String action, String payload) {
  if (action == "on") {
    // Enable TEMP_CONTROL mode
    controller.setCircleMode(circleIndex, CIRCLE_MODE_TEMP_CONTROL);
    publishCircleStatus(circleIndex, true);  // Force publish because mode changed
  } else if (action == "off") {
    // Disable circle (OFF mode)
    controller.setCircleMode(circleIndex, CIRCLE_MODE_OFF);
    publishCircleStatus(circleIndex, true);  // Force publish because mode changed
  } else {
    if (DEBUG_SERIAL) {
      Serial.println("❌ Unknown action: " + action);
    }
  }
}

void FloorHeatingManager::publishFullStatus() {
  if (!moduleManager || !moduleManager->getMQTTManager().isMQTTConnected()) {
    return;  // Don't publish if not connected
  }
  
  // Create JSON payload with all circles status
  // Increased size to handle full status with all circles (with margin for null values)
  StaticJsonDocument<2048> doc;
  doc["type"] = "full";
  
  JsonObject data = doc.createNestedObject("data");
  JsonObject circles = data.createNestedObject("circles");
  
  for (uint8_t i = 0; i < NUM_HEATING_CIRCLES; i++) {
    JsonObject circle = circles.createNestedObject(String(i));
    CircleMode mode = controller.getCircleMode(i);
    bool relayState = controller.getCircleState(i);
    bool hasError = sensors[i].hasSensorError();
    
    circle["mode"] = (mode == CIRCLE_MODE_OFF) ? "OFF" : "TEMP_CONTROL";
    circle["relay"] = relayState ? "ON" : "OFF";
    // Always set temperature to null when OFF mode or error (don't publish temperature)
    if (hasError || mode == CIRCLE_MODE_OFF) {
      circle["temperature"] = nullptr;  // JSON null
    } else {
      float temp = sensors[i].getLastTemperature();
      if (temp > 0 && !isnan(temp)) {
        float roundedTemp = round(temp);
        circle["temperature"] = roundedTemp;
        // Update last published temperature after successful publish
        sensors[i].setLastPublishedTemperature(roundedTemp);
      } else {
        circle["temperature"] = nullptr;  // JSON null
      }
    }
    circle["error"] = hasError;
  }
  
  String payload;
  serializeJson(doc, payload);
  
  String topic = "smartcamper/sensors/" + String(MODULE_ID) + "/status";
  moduleManager->getMQTTManager().publishRaw(topic, payload);
  
  if (DEBUG_MQTT) {
    Serial.println("📤 Published full floor heating status: " + topic);
  }
}

void FloorHeatingManager::publishCircleStatus(uint8_t circleIndex, bool forcePublish) {
  if (!moduleManager || !moduleManager->getMQTTManager().isMQTTConnected()) {
    return;  // Don't publish if not connected
  }
  
  if (circleIndex >= NUM_HEATING_CIRCLES) {
    return;
  }
  
  CircleMode mode = controller.getCircleMode(circleIndex);
  bool relayState = controller.getCircleState(circleIndex);
  bool hasError = sensors[circleIndex].hasSensorError();
  
  // Check if temperature has changed (only for TEMP_CONTROL mode and when not forcing publish)
  // This prevents unnecessary publishing when only temperature changes (relay/mode changes are always published)
  if (!forcePublish && mode == CIRCLE_MODE_TEMP_CONTROL && !hasError) {
    float currentTemp = sensors[circleIndex].getLastTemperature();
    float lastPublishedTemp = sensors[circleIndex].getLastPublishedTemperature();
    
    // Check if temperature is valid and different from last published
    if (currentTemp > 0 && !isnan(currentTemp) && !isnan(lastPublishedTemp)) {
      // Round both temperatures for comparison (same as in JSON)
      float roundedCurrent = round(currentTemp);
      float roundedLast = round(lastPublishedTemp);
      
      // If temperature hasn't changed, don't publish (unless forced)
      if (roundedCurrent == roundedLast) {
        if (DEBUG_MQTT) {
          Serial.println("⏭️ Skipping publish for circle " + String(circleIndex) + " - temperature unchanged: " + String(roundedCurrent) + "°C");
        }
        return;
      }
    }
  }
  
  // Create JSON payload for single circle
  StaticJsonDocument<256> doc;
  doc["type"] = "circle";
  doc["index"] = circleIndex;
  doc["mode"] = (mode == CIRCLE_MODE_OFF) ? "OFF" : "TEMP_CONTROL";
  doc["relay"] = relayState ? "ON" : "OFF";
  if (hasError) {
    doc["temperature"] = nullptr;  // JSON null
  } else if (mode == CIRCLE_MODE_OFF) {
    doc["temperature"] = nullptr;  // JSON null - don't publish temperature when OFF
  } else {
    float temp = sensors[circleIndex].getLastTemperature();
    if (temp > 0 && !isnan(temp)) {
      float roundedTemp = round(temp);
      doc["temperature"] = roundedTemp;
      
      // Update last published temperature after successful publish
      sensors[circleIndex].setLastPublishedTemperature(roundedTemp);
    } else {
      doc["temperature"] = nullptr;  // JSON null
    }
  }
  doc["error"] = hasError;
  
  String payload;
  serializeJson(doc, payload);
  
  String topic = "smartcamper/sensors/" + String(MODULE_ID) + "/status";
  moduleManager->getMQTTManager().publishRaw(topic, payload);
  
  if (DEBUG_MQTT) {
    Serial.println("📤 Published circle " + String(circleIndex) + " status: " + topic);
  }
}

void FloorHeatingManager::handleLevelingStart() {
  levelingSensor.start();
}

void FloorHeatingManager::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("📊 Floor Heating Manager Status:");
    controller.printStatus();
    for (uint8_t i = 0; i < NUM_HEATING_CIRCLES; i++) {
      sensors[i].printStatus();
    }
    buttonHandler.printStatus();
    levelingSensor.printStatus();
  }
}

