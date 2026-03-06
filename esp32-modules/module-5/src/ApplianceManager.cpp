// Appliance Manager Implementation
// Coordinator for all appliance-related functionality

#include "ApplianceManager.h"
#include "Config.h"
#include "MQTTManager.h"
#include <ArduinoJson.h>

// Static pointer to current instance
ApplianceManager* ApplianceManager::currentInstance = nullptr;

ApplianceManager::ApplianceManager(ModuleManager* moduleMgr) 
  : moduleManager(moduleMgr),
    relayController(),
    buttonHandler(&relayController),
    commandHandler(moduleMgr ? &moduleMgr->getMQTTManager() : nullptr, this, MODULE_ID),
    pendingStatusUpdate(false) {
  
  // Validate input parameter
  if (moduleMgr == nullptr) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ ERROR: ApplianceManager: moduleManager cannot be nullptr!");
    }
  }
  
  // Set current instance for static methods
  currentInstance = this;
}

void ApplianceManager::begin() {
  if (DEBUG_SERIAL) {
    Serial.println("🔌 Appliance Manager Starting...");
  }
  
  // Initialize relay controller
  relayController.begin();
  
  // Initialize button handler
  buttonHandler.setApplianceManager(this);  // Set ApplianceManager reference for status publishing
  buttonHandler.begin();
  
  // Set MQTT callback to ApplianceManager::handleMQTTMessage (handles both force_update and appliance commands)
  if (moduleManager) {
    moduleManager->getMQTTManager().setCallback(ApplianceManager::handleMQTTMessageStatic);
  }
  
  // Command handler will be initialized by ModuleManager
  // (ModuleManager.begin() is called with commandHandler reference)
  
  if (DEBUG_SERIAL) {
    Serial.println("✅ Appliance Manager Ready!");
  }
}

void ApplianceManager::loop() {
  // Update button handler
  buttonHandler.loop();
  
  // Process pending status update (deferred from MQTT callback)
  // This prevents blocking PubSubClient which can cause publish failures
  if (pendingStatusUpdate) {
    pendingStatusUpdate = false;
    publishFullStatus();
  }
}

void ApplianceManager::handleForceUpdate() {
  // Set flag to publish status in main loop (don't publish during MQTT callback)
  // This prevents blocking PubSubClient which can cause publish failures
  pendingStatusUpdate = true;
}

// Static MQTT callback method (wrapper for MQTTManager)
void ApplianceManager::handleMQTTMessageStatic(char* topic, byte* payload, unsigned int length) {
  if (currentInstance) {
    currentInstance->processMQTTMessage(topic, payload, length);
  }
}

// Process MQTT message (instance method)
void ApplianceManager::processMQTTMessage(char* topic, byte* payload, unsigned int length) {
  // First try force_update command (handled by CommandHandler)
  String topicStr = String(topic);
  if (topicStr.endsWith("/force_update")) {
    commandHandler.handleMQTTMessage(topic, payload, length);
    return;
  }
  
  // Handle appliance-specific commands
  processApplianceCommand(topic, payload, length);
}

// Process appliance-specific MQTT commands
void ApplianceManager::processApplianceCommand(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  String topicStr = String(topic);
  
  // Check if any button is pressed - ignore MQTT commands if so
  if (isAnyButtonPressed()) {
    if (DEBUG_SERIAL) {
      Serial.println("⚠️ Ignoring MQTT command - button is pressed");
    }
    return;
  }
  
  // Parse topic: smartcamper/commands/module-5/{command}
  String commandPrefix = String(MQTT_TOPIC_COMMANDS) + MODULE_ID + "/";
  
  if (!topicStr.startsWith(commandPrefix)) {
    return;  // Not our command
  }
  
  String commandPath = topicStr.substring(commandPrefix.length());
  
  // Handle relay commands: relay/{index}/toggle
  if (commandPath.startsWith("relay/")) {
    String relayCommand = commandPath.substring(6);  // Remove "relay/"
    
    // Extract relay index (first number)
    int slashIndex = relayCommand.indexOf('/');
    if (slashIndex == -1) {
      if (DEBUG_SERIAL) {
        Serial.println("❌ Invalid relay command format");
      }
      return;
    }
    
    String relayIndexStr = relayCommand.substring(0, slashIndex);
    uint8_t relayIndex = relayIndexStr.toInt();
    
    // Validate relay index
    if (relayIndex >= NUM_RELAYS) {
      if (DEBUG_SERIAL) {
        Serial.println("❌ Invalid relay index: " + String(relayIndex));
      }
      return;
    }
    
    String action = relayCommand.substring(slashIndex + 1);
    
    if (action == "toggle") {
      relayController.toggleRelay(relayIndex);
      publishRelayStatus();
    } else {
      if (DEBUG_SERIAL) {
        Serial.println("❌ Unknown action: " + action);
      }
    }
  }
}

void ApplianceManager::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("📊 Appliance Manager Status:");
    Serial.println("  Module Manager: " + String(moduleManager != nullptr ? "OK" : "NULL"));
    relayController.printStatus();
    buttonHandler.printStatus();
  }
}

// Publish full status (all relays)
void ApplianceManager::publishFullStatus() {
  if (!moduleManager || !moduleManager->isConnected()) {
    return;  // Cannot publish if not connected
  }
  
  // Create JSON object with all data
  StaticJsonDocument<512> doc;
  
  // Add data for all relays
  JsonObject relays = doc.createNestedObject("relays");
  for (uint8_t i = 0; i < NUM_RELAYS; i++) {
    JsonObject relay = relays.createNestedObject(String(i));
    relay["state"] = relayController.getRelayState(i) ? "ON" : "OFF";
  }
  
  // Serialize JSON
  String jsonString;
  serializeJson(doc, jsonString);
  
  // Publish in one topic
  String topic = String(MQTT_TOPIC_SENSORS) + MODULE_ID + "/status";
  moduleManager->getMQTTManager().publishRaw(topic, jsonString);
  
  if (DEBUG_VERBOSE && DEBUG_MQTT) {
    Serial.println("📤 Published full status: " + jsonString);
  }
}

void ApplianceManager::publishRelayStatus() {
  // On every relay change, send full status
  publishFullStatus();
}

bool ApplianceManager::isAnyButtonPressed() const {
  return buttonHandler.isAnyButtonPressed();
}
