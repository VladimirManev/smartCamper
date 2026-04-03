// LED Manager Implementation
// Coordinator for all LED-related functionality

#include "LEDManager.h"
#include "Config.h"
#include "MQTTManager.h"
#include <ArduinoJson.h>

// Static pointer to current instance
LEDManager* LEDManager::currentInstance = nullptr;

// Global pointer for CommandHandler force_update
LEDManager* g_ledManagerForForceUpdate = nullptr;

LEDManager::LEDManager(ModuleManager* moduleMgr) 
  : moduleManager(moduleMgr),
    ledStripController(moduleMgr),
    relayController(),
    buttonHandler(&ledStripController, &relayController),
    pirSensorHandler(&ledStripController),
    commandHandler(moduleMgr ? &moduleMgr->getMQTTManager() : nullptr, nullptr, MODULE_ID),
    pendingStatusUpdate(false) {
  
  // Validate input parameter
  if (moduleMgr == nullptr) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ ERROR: LEDManager: moduleManager cannot be nullptr!");
    }
  }
  
  // Set current instance for static methods
  currentInstance = this;
  g_ledManagerForForceUpdate = this;
}

void LEDManager::begin() {
  if (DEBUG_SERIAL) {
    Serial.println("💡 LED Manager Starting...");
  }
  
  // Initialize LED strip controller
  ledStripController.begin();
  
  // Register callback for strip state changes (for status publishing after delay)
  ledStripController.setStripStateChangeCallback(LEDManager::onStripStateChangedStatic);
  
  // Initialize relay controller
  relayController.begin();
  
  // Initialize button handler
  buttonHandler.setLEDManager(this);  // Set LEDManager reference for status publishing
  buttonHandler.begin();
  
  // Initialize PIR sensor handler
  pirSensorHandler.begin();
  
  // Set MQTT callback to LEDManager::handleMQTTMessage (handles both force_update and LED commands)
  if (moduleManager) {
    moduleManager->getMQTTManager().setCallback(LEDManager::handleMQTTMessageStatic);
  }
  
  // Command handler will be initialized by ModuleManager
  // (ModuleManager.begin() is called with commandHandler reference)
  
  if (DEBUG_SERIAL) {
    Serial.println("✅ LED Manager Ready!");
  }
}

void LEDManager::loop() {
  // Update LED strip controller (transitions, dimming, blinking)
  ledStripController.loop();
  
  // Update button handler
  buttonHandler.loop();
  
  // Update PIR sensor handler
  pirSensorHandler.loop();
  
  // Process pending status update (deferred from MQTT callback)
  // This prevents blocking PubSubClient which can cause publish failures
  if (pendingStatusUpdate) {
    pendingStatusUpdate = false;
    publishFullStatus();
  }
}

void LEDManager::handleForceUpdate() {
  // Set flag to publish status in main loop (don't publish during MQTT callback)
  // This prevents blocking PubSubClient which can cause publish failures
  pendingStatusUpdate = true;
}

// Static MQTT callback method (wrapper for MQTTManager)
void LEDManager::handleMQTTMessageStatic(char* topic, byte* payload, unsigned int length) {
  if (currentInstance) {
    currentInstance->processMQTTMessage(topic, payload, length);
  }
}

// Process MQTT message (instance method)
void LEDManager::processMQTTMessage(char* topic, byte* payload, unsigned int length) {
  // First try force_update command (handled by CommandHandler)
  String topicStr = String(topic);
  if (topicStr.endsWith("/force_update")) {
    commandHandler.handleMQTTMessage(topic, payload, length);
    return;
  }
  
  // Handle LED-specific commands
  processLEDCommand(topic, payload, length);
}

// Process LED-specific MQTT commands
void LEDManager::processLEDCommand(char* topic, byte* payload, unsigned int length) {
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
  
  // Parse topic: smartcamper/commands/module-2/{command}
  String commandPrefix = String(MQTT_TOPIC_COMMANDS) + MODULE_ID + "/";
  
  if (!topicStr.startsWith(commandPrefix)) {
    return;  // Not our command
  }
  
  String commandPath = topicStr.substring(commandPrefix.length());
  
  // Handle relay commands
  if (commandPath == "relay/toggle") {
    relayController.toggleRelay(0);
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
    if (stripIndex >= NUM_STRIPS) {
      if (DEBUG_SERIAL) {
        Serial.println("❌ Invalid strip index: " + String(stripIndex));
      }
      return;
    }
    
    String action = stripCommand.substring(slashIndex + 1);
    
    if (action == "on") {
      ledStripController.turnOnStrip(stripIndex);
      // Status will be published via callback after strip actually turns on
      // (handles power relay delay case)
    } else if (action == "off") {
      ledStripController.turnOffStrip(stripIndex);
      // Status will be published via callback
    } else if (action == "toggle") {
      ledStripController.toggleStrip(stripIndex);
      // Status will be published via callback
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
      
      ledStripController.setBrightnessSmooth(stripIndex, brightness);
      // Status is published when smooth transition completes (strip callback), not here —
      // immediate publish would send stale brightness while dimming is still running.
    } else if (action == "mode") {
      // Parse JSON payload: {"mode": "off"|"on"|"auto"} (case-insensitive)
      StaticJsonDocument<200> doc;
      DeserializationError error = deserializeJson(doc, message);
      
      if (error) {
        if (DEBUG_SERIAL) {
          Serial.println("❌ Failed to parse JSON: " + String(error.c_str()));
        }
        return;
      }
      
      if (!doc.containsKey("mode")) {
        if (DEBUG_SERIAL) {
          Serial.println("❌ Missing 'mode' field in JSON");
        }
        return;
      }
      
      String modeStr = doc["mode"].as<String>();
      modeStr.toLowerCase();
      StripMode mode;
      
      if (modeStr == "off") {
        mode = STRIP_MODE_OFF;
      } else if (modeStr == "on") {
        mode = STRIP_MODE_ON;
      } else if (modeStr == "auto") {
        mode = STRIP_MODE_AUTO;
      } else {
        if (DEBUG_SERIAL) {
          Serial.println("❌ Invalid mode: " + modeStr);
        }
        return;
      }
      
      ledStripController.setStripMode(stripIndex, mode);
      publishStripStatus(stripIndex);
    } else if (action == "apply") {
      StaticJsonDocument<768> adoc;
      DeserializationError aerr = deserializeJson(adoc, message);
      if (aerr) {
        if (DEBUG_SERIAL) {
          Serial.println("❌ apply JSON: " + String(aerr.c_str()));
        }
        return;
      }
      
      StripState& st = ledStripController.getStripState(stripIndex);
      bool needRedraw = false;
      
      if (adoc.containsKey("channels")) {
        JsonObject ch = adoc["channels"];
        if (ch.containsKey("r")) {
          st.chR = (uint8_t)constrain(ch["r"].as<int>(), 0, 255);
          needRedraw = true;
        }
        if (ch.containsKey("g")) {
          st.chG = (uint8_t)constrain(ch["g"].as<int>(), 0, 255);
          needRedraw = true;
        }
        if (ch.containsKey("b")) {
          st.chB = (uint8_t)constrain(ch["b"].as<int>(), 0, 255);
          needRedraw = true;
        }
        if (ch.containsKey("w")) {
          st.chW = (uint8_t)constrain(ch["w"].as<int>(), 0, 255);
          needRedraw = true;
        }
      }
      
      if (adoc.containsKey("effect")) {
        String ef = adoc["effect"].as<String>();
        ef.toLowerCase();
        st.effect = (ef == "rainbow_static") ? STRIP_EFFECT_RAINBOW_STATIC : STRIP_EFFECT_NORMAL;
        needRedraw = true;
      }
      
      if (adoc.containsKey("brightness")) {
        uint8_t b = (uint8_t)adoc["brightness"].as<unsigned int>();
        if (b < 1) b = 1;
        if (b > 255) b = 255;
        ledStripController.setBrightnessSmooth(stripIndex, b);
      }
      
      if (adoc.containsKey("mode")) {
        String ms = adoc["mode"].as<String>();
        ms.toLowerCase();
        StripMode nm = STRIP_MODE_OFF;
        if (ms == "on") {
          nm = STRIP_MODE_ON;
        } else if (ms == "auto") {
          nm = STRIP_MODE_AUTO;
        }
        ledStripController.setStripMode(stripIndex, nm);
      }
      
      if (adoc.containsKey("brightness")) {
        return;
      }
      
      if (needRedraw && st.on) {
        ledStripController.requestStripRedraw(stripIndex);
      }
      publishStripStatus(stripIndex);
    } else {
      if (DEBUG_SERIAL) {
        Serial.println("❌ Unknown action: " + action);
      }
    }
  }
}

void LEDManager::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("📊 LED Manager Status:");
    Serial.println("  Module Manager: " + String(moduleManager != nullptr ? "OK" : "NULL"));
    ledStripController.printStatus();
    relayController.printStatus();
    buttonHandler.printStatus();
    pirSensorHandler.printStatus();
  }
}

// Publish full status (all strips + relays)
void LEDManager::publishFullStatus() {
  if (!moduleManager || !moduleManager->isConnected()) {
    return;  // Cannot publish if not connected
  }
  
  StaticJsonDocument<1536> doc;
  
  JsonObject strips = doc.createNestedObject("strips");
  for (uint8_t i = 0; i < NUM_STRIPS; i++) {
    const StripState& state = ledStripController.getStripState(i);
    JsonObject strip = strips.createNestedObject(String(i));
    strip["state"] = state.on ? "ON" : "OFF";
    strip["brightness"] = state.brightness;
    
    const char* modeJson;
    if (state.mode == STRIP_MODE_OFF) {
      modeJson = "off";
    } else if (state.mode == STRIP_MODE_AUTO) {
      modeJson = "auto";
    } else {
      modeJson = "on";
    }
    strip["mode"] = modeJson;
    
    JsonObject ch = strip.createNestedObject("channels");
    ch["r"] = state.chR;
    ch["g"] = state.chG;
    ch["b"] = state.chB;
    ch["w"] = state.chW;
    strip["effect"] = (state.effect == STRIP_EFFECT_RAINBOW_STATIC) ? "rainbow_static" : "normal";
  }
  
  // Add data for all relays
  JsonObject relays = doc.createNestedObject("relays");
  for (uint8_t i = 0; i < NUM_RELAYS; i++) {
    JsonObject relay = relays.createNestedObject(String(i));
    relay["state"] = relayController.getRelayState(i) ? "ON" : "OFF";
  }
  
  // Serialize JSON
  String jsonString;
  serializeJson(doc, jsonString);
  
  // Publish in one topic (using module-2 instead of led-controller)
  String topic = String(MQTT_TOPIC_SENSORS) + MODULE_ID + "/status";
  moduleManager->getMQTTManager().publishRaw(topic, jsonString);
  
  if (DEBUG_VERBOSE && DEBUG_MQTT) {
    Serial.println("📤 Published full status: " + jsonString);
  }
}

void LEDManager::publishStripStatus(uint8_t stripIndex) {
  // On every strip change, send full status
  publishFullStatus();
}

void LEDManager::publishRelayStatus() {
  // On every relay change, send full status
  publishFullStatus();
}

bool LEDManager::isAnyButtonPressed() const {
  return buttonHandler.isAnyButtonPressed();
}

// Static callback wrapper for LEDStripController
void LEDManager::onStripStateChangedStatic(uint8_t stripIndex) {
  if (currentInstance) {
    currentInstance->onStripStateChanged(stripIndex);
  }
}

// Instance callback method - publishes status when strip state changes after delay
void LEDManager::onStripStateChanged(uint8_t stripIndex) {
  // Publish status after strip actually turned on (after power relay delay)
  publishStripStatus(stripIndex);
}


