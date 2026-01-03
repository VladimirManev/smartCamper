// Module Manager Implementation
// Common infrastructure for all ESP32 modules

#include "ModuleManager.h"
#include "CommandHandler.h"

ModuleManager::ModuleManager() 
  : heartbeatManager(&mqttManager, MODULE_ID) {
  this->commandHandler = nullptr;
  this->initialized = false;
}

void ModuleManager::begin() {
  begin(nullptr);
}

void ModuleManager::begin(CommandHandler* cmdHandler) {
  if (initialized) {
    return;  // Already initialized
  }
  
  Serial.begin(115200);
  Serial.println("🔧 Module " + String(MODULE_ID) + " Infrastructure Starting...");
  
  // Initialize network
  networkManager.begin();
  
  // Initialize MQTT
  mqttManager.begin();
  
  // Setup MQTT callback if command handler provided
  if (cmdHandler) {
    this->commandHandler = cmdHandler;
    mqttManager.setCallback(CommandHandler::handleMQTTMessageStatic);
    // Command handler begin() will be called after ModuleManager is ready
  }
  
  // Initialize Heartbeat Manager
  heartbeatManager.begin();
  
  // Initialize command handler if provided (after MQTT is ready)
  if (cmdHandler) {
    cmdHandler->begin();
  }
  
  initialized = true;
  Serial.println("✅ Module Infrastructure Ready!");
}

void ModuleManager::loop() {
  // Update network
  networkManager.loop();
  
  // Update MQTT with WiFi status
  bool wifiConnected = networkManager.isWiFiConnected();
  mqttManager.loop(wifiConnected);
  
  // Update Heartbeat Manager (must be after MQTT loop)
  heartbeatManager.loop();
  
  // Command Handler loop is handled internally if needed
}

bool ModuleManager::isConnected() {
  return networkManager.isWiFiConnected() && mqttManager.isMQTTConnected();
}

void ModuleManager::printStatus() {
  Serial.println("📊 Module Infrastructure Status:");
  Serial.println("  Initialized: " + String(initialized ? "Yes" : "No"));
  Serial.println("  WiFi: " + String(networkManager.isWiFiConnected() ? "Connected" : "Disconnected"));
  Serial.println("  IP: " + networkManager.getLocalIP());
  Serial.println("  Command Handler: " + String(commandHandler != nullptr ? "Set" : "Not set"));
  mqttManager.printStatus();
  heartbeatManager.printStatus();
}

