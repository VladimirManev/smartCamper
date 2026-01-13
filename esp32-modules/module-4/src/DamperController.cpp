// Damper Controller Implementation
// Controls a single damper with servo motor and button

#include "DamperController.h"
#include "DamperManager.h"
#include "MQTTManager.h"
#include <ArduinoJson.h>
#include <Arduino.h>

// Position definitions
const int DamperController::POSITIONS[NUM_POSITIONS] = {0, 45, 90};

DamperController::DamperController(int index, int servoPin, int btnPin, MQTTManager* mqtt, DamperManager* damperMgr)
  : servo(servoPin), buttonPin(btnPin), damperIndex(index), mqttManager(mqtt), damperManager(damperMgr),
    lastButtonState(false), debouncedButtonState(false), lastDebouncedState(false), lastDebounceTime(0),
    currentPositionIndex(2), lastPublishedAngle(-1) {  // Start at position 2 (90° - open)
}

void DamperController::begin() {
  // Initialize servo
  servo.begin();
  
  // Set initial position to 90° (open)
  servo.setAngle(90);
  currentPositionIndex = 2;  // Position 2 = 90° (open)
  
  // Initialize button pin
  pinMode(buttonPin, INPUT_PULLUP);
  lastButtonState = (digitalRead(buttonPin) == LOW);
  debouncedButtonState = lastButtonState;
  lastDebouncedState = lastButtonState;  // Initialize previous debounced state
  lastDebounceTime = millis();
  
  // Initialize last published angle to initial position (90°)
  lastPublishedAngle = 90;
  
  if (DEBUG_SERIAL) {
    Serial.println("🔧 DamperController " + String(damperIndex) + " initialized");
    Serial.println("  Button pin: " + String(buttonPin));
    Serial.println("  Initial position: 90° (open)");
  }
}

void DamperController::setAngle(int angle) {
  // Get current angle before setting new target
  int previousAngle = getCurrentAngle();
  
  // Safety check: verify that the change is allowed
  // Only check if damperManager is available and angle is actually changing
  if (damperManager != nullptr && angle != previousAngle) {
    if (!damperManager->checkCanChangeDamper(damperIndex, angle)) {
      // Safety check failed - block the change
      if (DEBUG_SERIAL) {
        Serial.println("🛡️ Safety check failed: Cannot change damper " + String(damperIndex) + 
                       " to " + String(angle) + "° (would leave insufficient dampers open)");
      }
      return;  // Block the change silently
    }
  }
  
  // Validate and set angle
  servo.setAngle(angle);
  
  // Update position index if angle matches a position
  for (int i = 0; i < NUM_POSITIONS; i++) {
    if (POSITIONS[i] == angle) {
      currentPositionIndex = i;
      break;
    }
  }
  
  // If servo is already at target (no movement needed), publish status immediately
  // Otherwise, status will be published in loop() when servo reaches target
  if (!servo.isMovingToTarget() && getCurrentAngle() == angle) {
    // Servo already at target - publish status immediately
    publishStatus();
  }
}

void DamperController::togglePosition() {
  // Cycle through positions: 90° → 45° → 0° → 90°
  // Position indices: 2 (90°) → 1 (45°) → 0 (0°) → 2 (90°)
  int nextPositionIndex;
  if (currentPositionIndex == 2) {  // 90° -> 45°
    nextPositionIndex = 1;
  } else if (currentPositionIndex == 1) {  // 45° -> 0°
    nextPositionIndex = 0;
  } else {  // 0° -> 90°
    nextPositionIndex = 2;
  }
  
  currentPositionIndex = nextPositionIndex;
  int newAngle = POSITIONS[currentPositionIndex];
  
  if (DEBUG_SERIAL) {
    Serial.println("🔄 Damper " + String(damperIndex) + ": Toggling to position " + String(currentPositionIndex) + " (" + String(newAngle) + "°)");
  }
  
  setAngle(newAngle);
}

void DamperController::processButton() {
  unsigned long currentTime = millis();
  bool rawButtonReading = (digitalRead(buttonPin) == LOW);
  
  // Debounce logic: reset timer if button state changed
  if (rawButtonReading != lastButtonState) {
    lastDebounceTime = currentTime;
  }
  
  // Update debounced state only after debounce delay
  if (currentTime - lastDebounceTime > DEBOUNCE_DELAY) {
    // Only update if state actually changed
    if (debouncedButtonState != rawButtonReading) {
      debouncedButtonState = rawButtonReading;
    }
  }
  
  lastButtonState = rawButtonReading;
  
  // Detect button press (rising edge: transition from not-pressed to pressed)
  // Only trigger on rising edge (button was released, now pressed)
  if (debouncedButtonState && !lastDebouncedState) {
    // Button pressed - toggle position
    togglePosition();
  }
  
  // Update previous debounced state
  lastDebouncedState = debouncedButtonState;
}

void DamperController::publishStatus() {
  if (!mqttManager || !mqttManager->isMQTTConnected()) {
    return;  // Cannot publish if MQTT not connected
  }
  
  int currentAngle = getCurrentAngle();
  
  // Create JSON payload
  StaticJsonDocument<128> doc;
  doc["angle"] = currentAngle;
  
  String payload;
  serializeJson(doc, payload);
  
  // Publish to MQTT
  String topic = MQTT_TOPIC_SENSORS + String("module-4/damper/") + String(damperIndex) + "/angle";
  bool success = mqttManager->publishRaw(topic, payload);
  
  if (DEBUG_MQTT) {
    if (success) {
      Serial.println("📤 Published damper " + String(damperIndex) + " status: " + payload);
    } else {
      Serial.println("❌ Failed to publish damper " + String(damperIndex) + " status");
    }
  }
}

int DamperController::getCurrentAngle() const {
  return servo.getCurrentAngle();
}

void DamperController::loop() {
  // Update servo (for smooth movement)
  bool wasMoving = servo.isMovingToTarget();
  servo.loop();
  
  // Process button
  processButton();
  
  // Publish status when servo reaches target (was moving, now stopped)
  if (wasMoving && servo.isAtTarget()) {
    // Servo was moving and now reached target - publish status
    int currentAngle = getCurrentAngle();
    if (lastPublishedAngle != currentAngle) {
      publishStatus();
      lastPublishedAngle = currentAngle;
    }
  }
}

void DamperController::forceUpdate() {
  publishStatus();
}

void DamperController::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("📊 DamperController " + String(damperIndex) + " Status:");
    Serial.println("  Current Angle: " + String(getCurrentAngle()) + "°");
    Serial.println("  Position Index: " + String(currentPositionIndex));
    Serial.println("  Button Pin: " + String(buttonPin));
    servo.printStatus();
  }
}

