/**
 * @file main.cpp
 * @brief Module 7 - Clean Water Level Sensor
 *
 * Dedicated ESP32 module for clean (fresh) water tank level measurement
 * using conductivity-based electrodes.
 */

#include "Config.h"
#include "ModuleManager.h"
#include "CleanWaterLevelManager.h"

ModuleManager moduleManager;
CleanWaterLevelManager cleanWaterLevelManager(&moduleManager);

void setup() {
  moduleManager.begin(&cleanWaterLevelManager.getCommandHandler());

  if (!moduleManager.isInitialized()) {
    if (DEBUG_SERIAL) {
      Serial.println("ERROR: ModuleManager failed to initialize!");
    }
    return;
  }

  cleanWaterLevelManager.begin();

  if (DEBUG_SERIAL) {
    Serial.println("Module 7 ready");
  }
}

void loop() {
  moduleManager.loop();
  cleanWaterLevelManager.loop();
  delay(10);
}
