// Damper Manager Implementation
// Coordinates all damper controllers

#include "DamperManager.h"
#include "Config.h"
#include <ArduinoJson.h>
#include <Arduino.h>

// Static pointer to current instance
DamperManager* DamperManager::currentInstance = nullptr;

DamperManager::DamperManager(ModuleManager* moduleMgr)
  : moduleManager(moduleMgr), dampers(nullptr), numDampers(NUM_DAMPERS) {
  
  // Validate input
  if (moduleMgr == nullptr) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ ERROR: DamperManager: moduleManager cannot be nullptr!");
    }
    return;
  }
  
  // Allocate array of pointers
  dampers = new DamperController*[numDampers];
  
  // Initialize each damper with its pins
  // Note: For now we have only one damper, but structure allows easy expansion
  if (numDampers >= 1) {
    dampers[0] = new DamperController(0, DAMPER_0_SERVO_PIN, DAMPER_0_BUTTON_PIN, &moduleMgr->getMQTTManager());
  }
  // Add more dampers here as needed:
  // if (numDampers >= 2) {
  //   dampers[1] = new DamperController(1, DAMPER_1_SERVO_PIN, DAMPER_1_BUTTON_PIN, &moduleMgr->getMQTTManager());
  // }
  
  // Set current instance for static methods
  currentInstance = this;
}

DamperManager::~DamperManager() {
  if (dampers) {
    // Delete each damper object
    for (int i = 0; i < numDampers; i++) {
      if (dampers[i]) {
        delete dampers[i];
      }
    }
    // Delete array of pointers
    delete[] dampers;
  }
}

void DamperManager::begin() {
  if (DEBUG_SERIAL) {
    Serial.println("🌬️ Damper Manager Starting...");
    Serial.println("  Number of dampers: " + String(numDampers));
  }
  
  // Initialize all dampers
  for (int i = 0; i < numDampers; i++) {
    if (dampers[i]) {
      dampers[i]->begin();
    }
  }
  
  if (DEBUG_SERIAL) {
    Serial.println("✅ Damper Manager Ready!");
  }
}

void DamperManager::loop() {
  // Update all dampers
  for (int i = 0; i < numDampers; i++) {
    if (dampers[i]) {
      dampers[i]->loop();
    }
  }
}

void DamperManager::handleMQTTCommand(const String& commandJson) {
  // Parse JSON command
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, commandJson);
  
  if (error) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ Failed to parse damper command JSON: " + String(error.c_str()));
    }
    return;
  }
  
  // Validate command structure
  if (!doc.containsKey("type") || !doc.containsKey("index") || !doc.containsKey("action")) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ Invalid damper command: missing required fields");
    }
    return;
  }
  
  String type = doc["type"].as<String>();
  if (type != "damper") {
    return;  // Not a damper command
  }
  
  int index = doc["index"].as<int>();
  String action = doc["action"].as<String>();
  
  // Validate index
  if (index < 0 || index >= numDampers) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ Invalid damper index: " + String(index));
    }
    return;
  }
  
  // Handle action
  if (action == "set_angle") {
    if (!doc.containsKey("angle")) {
      if (DEBUG_SERIAL) {
        Serial.println("❌ set_angle command missing 'angle' field");
      }
      return;
    }
    
    int angle = doc["angle"].as<int>();
    if (DEBUG_SERIAL) {
      Serial.println("📨 Received damper command: index=" + String(index) + ", angle=" + String(angle) + "°");
    }
    
    if (dampers[index]) {
      dampers[index]->setAngle(angle);
    }
  } else {
    if (DEBUG_SERIAL) {
      Serial.println("❌ Unknown damper action: " + action);
    }
  }
}

void DamperManager::handleMQTTCommandStatic(const String& commandJson) {
  if (currentInstance) {
    currentInstance->handleMQTTCommand(commandJson);
  }
}

void DamperManager::forceUpdate() {
  if (DEBUG_SERIAL) {
    Serial.println("🔄 Force update: Publishing all damper statuses");
  }
  
  // Publish status for all dampers
  for (int i = 0; i < numDampers; i++) {
    if (dampers[i]) {
      dampers[i]->forceUpdate();
    }
  }
}

void DamperManager::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("📊 Damper Manager Status:");
    Serial.println("  Number of dampers: " + String(numDampers));
    for (int i = 0; i < numDampers; i++) {
      if (dampers[i]) {
        dampers[i]->printStatus();
      }
    }
  }
}

