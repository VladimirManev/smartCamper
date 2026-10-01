/**
 * Scratch pad — production Victron + fridge is in esp32-modules/module-6.
 * Ducato CAN decode notes: repo root CAN_SIGNALS.md
 */

#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("esp32-modules/test — empty scratch. Use module-6 for Victron+fridge.");
}

void loop() {
  delay(1000);
}
