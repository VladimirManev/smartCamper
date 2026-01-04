// Relay Controller Implementation

#include "RelayController.h"

RelayController::RelayController() {
  for (uint8_t i = 0; i < NUM_RELAYS; i++) {
    relayStates[i] = false;
  }
}

void RelayController::begin() {
  if (DEBUG_SERIAL) {
    Serial.println("🔌 Relay Controller Starting...");
  }
  
  // Initialize relay pins
  for (uint8_t i = 0; i < NUM_RELAYS; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], LOW);  // Start with relay OFF
    relayStates[i] = false;
    
    if (DEBUG_SERIAL) {
      Serial.println("Relay " + String(i) + " - Pin: " + String(relayPins[i]) + " - OK (initialized OFF)");
    }
  }
  
  if (DEBUG_SERIAL) {
    Serial.println("✅ Relay Controller Ready!");
  }
}

void RelayController::toggleRelay(uint8_t relayIndex) {
  if (relayIndex >= NUM_RELAYS) return;
  
  relayStates[relayIndex] = !relayStates[relayIndex];
  digitalWrite(relayPins[relayIndex], relayStates[relayIndex] ? HIGH : LOW);
  
  Serial.println("🔌 Relay " + String(relayIndex) + " " + String(relayStates[relayIndex] ? "ON" : "OFF") + 
                 " (Pin " + String(relayPins[relayIndex]) + ")");
}

void RelayController::setRelay(uint8_t relayIndex, bool state) {
  if (relayIndex >= NUM_RELAYS) return;
  
  if (relayStates[relayIndex] != state) {
    relayStates[relayIndex] = state;
    digitalWrite(relayPins[relayIndex], state ? HIGH : LOW);
    
    Serial.println("🔌 Relay " + String(relayIndex) + " " + String(state ? "ON" : "OFF") + 
                   " (Pin " + String(relayPins[relayIndex]) + ")");
  }
}

bool RelayController::getRelayState(uint8_t relayIndex) const {
  if (relayIndex >= NUM_RELAYS) return false;
  return relayStates[relayIndex];
}

void RelayController::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("📊 Relay Controller Status:");
    for (uint8_t i = 0; i < NUM_RELAYS; i++) {
      Serial.println("  Relay " + String(i) + ": " + String(relayStates[i] ? "ON" : "OFF") + 
                     " (Pin " + String(relayPins[i]) + ")");
    }
  }
}

