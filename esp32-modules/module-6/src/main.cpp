/**
 * @file main.cpp
 * @brief Module 6 - Victron BLE Energy Monitor + AAOBOSI fridge GATT
 *
 * Victron Instant Readout ads → MQTT status every 2s.
 * Fridge Alpicool GATT → MQTT fridge status + mode/zone commands.
 * Keep ESP32 within 1–3 m of Victron devices and the fridge.
 * Close the phone fridge app (one BLE client only).
 */

#include "Config.h"
#include "ModuleManager.h"
#include "VictronManager.h"
#include "FridgeManager.h"

ModuleManager moduleManager;
VictronManager victronManager(&moduleManager);
FridgeManager fridgeManager(&moduleManager, &victronManager);

void setup() {
  moduleManager.begin(&victronManager.getCommandHandler());

  if (!moduleManager.isInitialized()) {
    if (DEBUG_SERIAL) {
      Serial.println("ERROR: ModuleManager failed to initialize!");
    }
    return;
  }

  victronManager.begin();
  fridgeManager.begin();
  victronManager.getCommandHandler().setFridgeManager(&fridgeManager);

  if (DEBUG_SERIAL) {
    Serial.println("Module 6 ready (Victron + fridge BLE)");
  }
}

void loop() {
  moduleManager.loop();
  victronManager.loop();
  fridgeManager.loop();
  delay(10);
}
