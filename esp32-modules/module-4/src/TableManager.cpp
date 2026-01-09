// Table Manager Implementation
// Coordinates table controller

#include "TableManager.h"
#include "Config.h"
#include <ArduinoJson.h>
#include <Arduino.h>

// Static pointer to current instance
TableManager* TableManager::currentInstance = nullptr;

TableManager::TableManager(ModuleManager* moduleMgr)
  : moduleManager(moduleMgr), tableController(nullptr) {
  
  // Validate input
  if (moduleMgr == nullptr) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ ERROR: TableManager: moduleManager cannot be nullptr!");
    }
    return;
  }
  
  // Initialize table controller
  tableController = new TableController(
    TABLE_RELAY_UP_PIN,
    TABLE_RELAY_DOWN_PIN,
    TABLE_BUTTON_UP_PIN,
    TABLE_BUTTON_DOWN_PIN,
    &moduleMgr->getMQTTManager()
  );
  
  // Set current instance for static methods
  currentInstance = this;
}

TableManager::~TableManager() {
  if (tableController) {
    delete tableController;
  }
}

void TableManager::begin() {
  if (DEBUG_SERIAL) {
    Serial.println("🪑 Table Manager Starting...");
  }
  
  // Initialize table controller
  if (tableController) {
    tableController->begin();
  }
  
  if (DEBUG_SERIAL) {
    Serial.println("✅ Table Manager Ready!");
  }
}

void TableManager::loop() {
  // Update table controller
  if (tableController) {
    tableController->loop();
  }
}

void TableManager::handleMQTTCommand(const String& commandJson) {
  // Parse JSON command
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, commandJson);
  
  if (error) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ Failed to parse table command JSON: " + String(error.c_str()));
    }
    return;
  }
  
  // Validate command structure
  if (!doc.containsKey("type") || !doc.containsKey("action")) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ Invalid table command: missing required fields");
    }
    return;
  }
  
  String type = doc["type"].as<String>();
  if (type != "table") {
    return;  // Not a table command
  }
  
  String action = doc["action"].as<String>();
  
  if (!tableController) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ TableController not initialized");
    }
    return;
  }
  
  // Handle actions
  if (action == "move_up") {
    tableController->moveUp();
    if (DEBUG_SERIAL) {
      Serial.println("📨 Received table command: move_up");
    }
  } else if (action == "move_down") {
    tableController->moveDown();
    if (DEBUG_SERIAL) {
      Serial.println("📨 Received table command: move_down");
    }
  } else if (action == "stop") {
    tableController->stop();
    if (DEBUG_SERIAL) {
      Serial.println("📨 Received table command: stop");
    }
  } else if (action == "move_up_auto") {
    int duration = doc.containsKey("duration") ? doc["duration"].as<int>() : TABLE_AUTO_MOVE_DURATION;
    tableController->moveUpAuto(duration);
    if (DEBUG_SERIAL) {
      Serial.println("📨 Received table command: move_up_auto (duration: " + String(duration) + "ms)");
    }
  } else if (action == "move_down_auto") {
    int duration = doc.containsKey("duration") ? doc["duration"].as<int>() : TABLE_AUTO_MOVE_DURATION;
    tableController->moveDownAuto(duration);
    if (DEBUG_SERIAL) {
      Serial.println("📨 Received table command: move_down_auto (duration: " + String(duration) + "ms)");
    }
  } else {
    if (DEBUG_SERIAL) {
      Serial.println("❌ Unknown table action: " + action);
    }
  }
}

void TableManager::handleMQTTCommandStatic(const String& commandJson) {
  if (currentInstance) {
    currentInstance->handleMQTTCommand(commandJson);
  }
}

void TableManager::forceUpdate() {
  if (DEBUG_SERIAL) {
    Serial.println("🔄 Force update: Publishing table status");
  }
  
  if (tableController) {
    tableController->forceUpdate();
  }
}

void TableManager::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("📊 Table Manager Status:");
    if (tableController) {
      tableController->printStatus();
    }
  }
}

