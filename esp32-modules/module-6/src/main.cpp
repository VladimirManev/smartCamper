/**
 * @file main.cpp
 * @brief Module 6 - Victron BLE Energy Monitor
 *
 * Reads Victron Instant Readout BLE advertisements and publishes
 * a full energy snapshot to MQTT every 2 seconds.
 */

#include "Config.h"
#include "ModuleManager.h"
#include "VictronManager.h"

ModuleManager moduleManager;
VictronManager victronManager(&moduleManager);

void setup() {
  moduleManager.begin(&victronManager.getCommandHandler());

  if (!moduleManager.isInitialized()) {
    if (DEBUG_SERIAL) {
      Serial.println("ERROR: ModuleManager failed to initialize!");
    }
    return;
  }

  victronManager.begin();

  if (DEBUG_SERIAL) {
    Serial.println("Module 6 ready (BLE deferred until network connect)");
  }
}

void loop() {
  moduleManager.loop();
  victronManager.loop();
  delay(10);
}
