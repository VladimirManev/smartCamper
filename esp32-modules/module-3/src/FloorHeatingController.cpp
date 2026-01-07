// Floor Heating Controller Implementation

#include "FloorHeatingController.h"
#include "FloorHeatingSensor.h"
#include "FloorHeatingManager.h"
#include <Arduino.h>

FloorHeatingController::FloorHeatingController() {
  // Initialize relay pins array
  relayPins[0] = HEATING_RELAY_PIN_0;
  relayPins[1] = HEATING_RELAY_PIN_1;
  relayPins[2] = HEATING_RELAY_PIN_2;
  relayPins[3] = HEATING_RELAY_PIN_3;
  
  // Initialize states
  for (uint8_t i = 0; i < NUM_HEATING_CIRCLES; i++) {
    relayStates[i] = false;
    circleModes[i] = CIRCLE_MODE_OFF;  // Start with all circles OFF
    sensors[i] = nullptr;
    lastControlCheck[i] = 0;
  }
  
  manager = nullptr;
  
  // Initialize temperature settings
  targetTemperature = HEATING_TARGET_TEMP;
  turnOffTemperature = HEATING_TURN_OFF_TEMP;
  turnOnTemperature = HEATING_TURN_ON_TEMP;
}

void FloorHeatingController::begin() {
  if (DEBUG_SERIAL) {
    Serial.println("🔥 Floor Heating Controller Starting...");
  }
  
  // Initialize relay pins
  for (uint8_t i = 0; i < NUM_HEATING_CIRCLES; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], LOW);  // Start with relay OFF
    relayStates[i] = false;
    
    if (DEBUG_SERIAL) {
      Serial.println("Circle " + String(i) + " - Relay Pin: " + String(relayPins[i]) + " - OK (initialized OFF)");
    }
  }
  
  if (DEBUG_SERIAL) {
    Serial.println("✅ Floor Heating Controller Ready!");
    Serial.println("   Target Temperature: " + String(targetTemperature) + "°C");
    Serial.println("   Turn Off Temperature: " + String(turnOffTemperature) + "°C");
    Serial.println("   Turn On Temperature: " + String(turnOnTemperature) + "°C");
  }
}

void FloorHeatingController::setSensor(uint8_t circleIndex, FloorHeatingSensor* sensor) {
  if (circleIndex >= NUM_HEATING_CIRCLES) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ ERROR: setSensor: circleIndex out of range!");
    }
    return;
  }
  
  sensors[circleIndex] = sensor;
  
  if (DEBUG_SERIAL) {
    Serial.println("✅ Floor Heating Controller: Sensor set for circle " + String(circleIndex));
  }
}

void FloorHeatingController::setManager(FloorHeatingManager* mgr) {
  manager = mgr;
}

void FloorHeatingController::loop() {
  unsigned long currentTime = millis();
  
  // Check each circle for temperature control (only if in TEMP_CONTROL mode)
  for (uint8_t i = 0; i < NUM_HEATING_CIRCLES; i++) {
    // Only check if circle is in TEMP_CONTROL mode
    if (circleModes[i] == CIRCLE_MODE_TEMP_CONTROL) {
      // Check temperature every HEATING_MEASURE_INTERVAL (30 seconds)
      // OR immediately if lastControlCheck is 0 (reset by sensor when new reading is available)
      if (lastControlCheck[i] == 0 || (currentTime - lastControlCheck[i] >= HEATING_MEASURE_INTERVAL)) {
        lastControlCheck[i] = currentTime;
        updateCircleControl(i);
      }
    }
  }
}

void FloorHeatingController::updateCircleControl(uint8_t circleIndex) {
  if (circleIndex >= NUM_HEATING_CIRCLES) {
    return;
  }
  
  // Check if sensor is available
  if (sensors[circleIndex] == nullptr) {
    if (DEBUG_SERIAL) {
      Serial.println("⚠️ WARNING: No sensor available for circle " + String(circleIndex));
    }
    return;
  }
  
  // Get current temperature
  float currentTemp = sensors[circleIndex]->getLastTemperature();
  
  // Check if temperature is valid
  if (isnan(currentTemp) || currentTemp == 0.0) {
    // Invalid temperature - don't change state
    if (DEBUG_SERIAL) {
      Serial.println("⚠️ WARNING: Invalid temperature for circle " + String(circleIndex) + ", keeping current state");
    }
    return;
  }
  
  // Automatic control logic with hysteresis
  bool stateChanged = false;
  if (relayStates[circleIndex]) {
    // Relay is ON - check if we should turn it OFF
    if (currentTemp >= turnOffTemperature) {
      bool oldState = relayStates[circleIndex];
      setRelayState(circleIndex, false);
      stateChanged = (oldState != relayStates[circleIndex]);
      if (DEBUG_SERIAL) {
        Serial.println("🔥 Circle " + String(circleIndex) + " OFF (temp: " + String(currentTemp, 1) + "°C >= " + String(turnOffTemperature) + "°C)");
      }
    }
  } else {
    // Relay is OFF - check if we should turn it ON
    if (currentTemp < turnOnTemperature) {
      bool oldState = relayStates[circleIndex];
      setRelayState(circleIndex, true);
      stateChanged = (oldState != relayStates[circleIndex]);
      if (DEBUG_SERIAL) {
        Serial.println("🔥 Circle " + String(circleIndex) + " ON (temp: " + String(currentTemp, 1) + "°C < " + String(turnOnTemperature) + "°C)");
      }
    }
  }
  
  // Publish status if state changed (via manager callback)
  // Force publish because relay state changed, not just temperature
  if (stateChanged && manager != nullptr) {
    manager->publishCircleStatus(circleIndex, true);
  }
}

void FloorHeatingController::setRelayState(uint8_t circleIndex, bool state) {
  if (circleIndex >= NUM_HEATING_CIRCLES) {
    return;
  }
  
  if (relayStates[circleIndex] != state) {
    relayStates[circleIndex] = state;
    digitalWrite(relayPins[circleIndex], state ? HIGH : LOW);
    
    if (DEBUG_SERIAL) {
      Serial.println("🔥 Circle " + String(circleIndex) + " relay " + String(state ? "ON" : "OFF") + 
                     " (Pin " + String(relayPins[circleIndex]) + ")");
    }
  }
}

void FloorHeatingController::setCircleMode(uint8_t circleIndex, CircleMode mode) {
  if (circleIndex >= NUM_HEATING_CIRCLES) {
    return;
  }
  
  if (circleModes[circleIndex] != mode) {
    circleModes[circleIndex] = mode;
    
    // If switching to OFF mode, turn off relay
    if (mode == CIRCLE_MODE_OFF) {
      setRelayState(circleIndex, false);
    }
    // If switching to TEMP_CONTROL mode, reset last check time
    // This will trigger immediate check in next loop() iteration
    // We don't call updateCircleControl here because temperature might not be ready yet
    // Instead, we reset the check time so it will be checked as soon as temperature is available
    else if (mode == CIRCLE_MODE_TEMP_CONTROL) {
      // Reset last check time to force immediate check in next loop()
      lastControlCheck[circleIndex] = 0;
    }
    
    if (DEBUG_SERIAL) {
      String modeStr = (mode == CIRCLE_MODE_OFF) ? "OFF" : "TEMP_CONTROL";
      Serial.println("🔥 Circle " + String(circleIndex) + " mode set to " + modeStr);
    }
    
    // Publish status update
    // Force publish because circle mode changed
    if (manager != nullptr) {
      manager->publishCircleStatus(circleIndex, true);
    }
  }
}

void FloorHeatingController::toggleCircleMode(uint8_t circleIndex) {
  if (circleIndex >= NUM_HEATING_CIRCLES) {
    return;
  }
  
  // Toggle between OFF and TEMP_CONTROL
  CircleMode newMode = (circleModes[circleIndex] == CIRCLE_MODE_OFF) ? CIRCLE_MODE_TEMP_CONTROL : CIRCLE_MODE_OFF;
  setCircleMode(circleIndex, newMode);
}

bool FloorHeatingController::getCircleState(uint8_t circleIndex) const {
  if (circleIndex >= NUM_HEATING_CIRCLES) {
    return false;
  }
  return relayStates[circleIndex];
}

CircleMode FloorHeatingController::getCircleMode(uint8_t circleIndex) const {
  if (circleIndex >= NUM_HEATING_CIRCLES) {
    return CIRCLE_MODE_OFF;
  }
  return circleModes[circleIndex];
}

void FloorHeatingController::resetLastCheckTime(uint8_t circleIndex) {
  if (circleIndex >= NUM_HEATING_CIRCLES) {
    return;
  }
  lastControlCheck[circleIndex] = 0;  // Reset to 0 to force immediate check
}

void FloorHeatingController::setTargetTemperature(float temp) {
  // Future feature: Set target temperature
  // For now, we use fixed values from Config.h
  // This method is prepared for future implementation
  targetTemperature = temp;
  turnOffTemperature = targetTemperature;
  turnOnTemperature = targetTemperature - HEATING_HYSTERESIS;
  
  if (DEBUG_SERIAL) {
    Serial.println("🔥 Target temperature set to " + String(targetTemperature) + "°C");
    Serial.println("   Turn Off: " + String(turnOffTemperature) + "°C");
    Serial.println("   Turn On: " + String(turnOnTemperature) + "°C");
  }
}

void FloorHeatingController::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("📊 Floor Heating Controller Status:");
    for (uint8_t i = 0; i < NUM_HEATING_CIRCLES; i++) {
      String modeStr = (circleModes[i] == CIRCLE_MODE_OFF) ? "OFF" : "TEMP_CONTROL";
      float temp = (sensors[i] != nullptr) ? sensors[i]->getLastTemperature() : 0.0;
      Serial.println("  Circle " + String(i) + ": " + String(relayStates[i] ? "ON" : "OFF") + 
                     " (" + modeStr + ") - Temp: " + String(temp, 1) + "°C (Pin " + String(relayPins[i]) + ")");
    }
  }
}

