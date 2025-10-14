// Temperature Sensor Module - Main
// ESP32 модул за температура и влажност с симулирани данни

#include "Config.h"
#include "SensorManager.h"

SensorManager sensorManager;

void setup() {
  sensorManager.begin();
}

void loop() {
  sensorManager.loop();
}
