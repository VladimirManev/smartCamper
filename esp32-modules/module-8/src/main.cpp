/**
 * @file main.cpp
 * @brief Module 8 - Security Alarm
 *
 * Autonomous interior + perimeter alarm with MQTT status/commands.
 */

#include "Config.h"
#include "ModuleManager.h"
#include "AlarmManager.h"

ModuleManager moduleManager;
AlarmManager alarmManager(&moduleManager);

void setup() {
  moduleManager.begin(&alarmManager.getCommandHandler());

  if (!moduleManager.isInitialized()) {
    if (DEBUG_SERIAL) {
      Serial.println("ERROR: ModuleManager failed to initialize!");
    }
    return;
  }

  alarmManager.begin();

  if (DEBUG_SERIAL) {
    Serial.println("Module 8 ready");
  }
}

void loop() {
  moduleManager.loop();
  alarmManager.loop();
  delay(10);
}
