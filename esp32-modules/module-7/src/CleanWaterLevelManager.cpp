// Clean Water Level Manager Implementation

#include "CleanWaterLevelManager.h"

CleanWaterLevelManager::CleanWaterLevelManager(ModuleManager* moduleMgr)
  : moduleManager(moduleMgr),
    cleanWaterLevelSensor(moduleMgr ? &moduleMgr->getMQTTManager() : nullptr),
    commandHandler(moduleMgr ? &moduleMgr->getMQTTManager() : nullptr, this, MODULE_ID) {
  if (moduleMgr == nullptr && DEBUG_SERIAL) {
    Serial.println("ERROR: CleanWaterLevelManager: moduleManager cannot be nullptr!");
  }
}

void CleanWaterLevelManager::begin() {
  if (DEBUG_SERIAL) {
    Serial.println("Clean Water Level Manager starting...");
  }

  cleanWaterLevelSensor.begin();

  if (DEBUG_SERIAL) {
    Serial.println("Clean Water Level Manager ready");
  }
}

void CleanWaterLevelManager::loop() {
  cleanWaterLevelSensor.loop();
}

void CleanWaterLevelManager::handleForceUpdate() {
  cleanWaterLevelSensor.forceUpdate();
}

void CleanWaterLevelManager::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("Clean Water Level Manager Status:");
    cleanWaterLevelSensor.printStatus();
  }
}
