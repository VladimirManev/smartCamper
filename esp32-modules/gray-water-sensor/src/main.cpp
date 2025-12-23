// Gray Water Level Sensor Module - Main
// ESP32 модул за следене на нивото на сива вода

#include "Config.h"
#include "WaterLevelManager.h"

WaterLevelManager waterLevelManager;

void setup() {
  waterLevelManager.begin();
}

void loop() {
  waterLevelManager.loop();
}

