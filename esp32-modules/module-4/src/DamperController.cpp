// Damper Controller Implementation
// Controls a single damper with servo motor and button

#include "DamperController.h"
#include "MQTTManager.h"
#include <ArduinoJson.h>
#include <Arduino.h>

// Position definitions
const int DamperController::POSITIONS[NUM_POSITIONS] = {0, 45, 90};

DamperController::DamperController(int index, int servoPin, int btnPin, MQTTManager* mqtt)
  : servo(servoPin), buttonPin(btnPin), damperIndex(index), mqttManager(mqtt),
    lastButtonState(false), debouncedButtonState(false), lastDebounceTime(0),
    currentPositionIndex(0) {
}

void DamperController::begin() {
  // Initialize servo
  servo.begin();
  
  // Set initial position to 0°
  servo.setAngle(0);
  currentPositionIndex = 0;
  
  // Initialize button pin
  pinMode(buttonPin, INPUT_PULLUP);
  lastButtonState = (digitalRead(buttonPin) == LOW);
  debouncedButtonState = lastButtonState;
  lastDebounceTime = millis();
  
  if (DEBUG_SERIAL) {
    Serial.println("🔧 DamperController " + String(damperIndex) + " initialized");
    Serial.println("  Button pin: " + String(buttonPin));
    Serial.println("  Initial position: 0°");
  }
}

void DamperController::setAngle(int angle) {
  // Validate and set angle
  servo.setAngle(angle);
  
  // Update position index if angle matches a position
  for (int i = 0; i < NUM_POSITIONS; i++) {
    if (POSITIONS[i] == angle) {
      currentPositionIndex = i;
      break;
    }
  }
  
  // Status will be published in loop() when servo reaches target
}

void DamperController::togglePosition() {
  // Cycle through positions: 0 → 1 → 2 → 0
  currentPositionIndex = (currentPositionIndex + 1) % NUM_POSITIONS;
  int newAngle = POSITIONS[currentPositionIndex];
  
  if (DEBUG_SERIAL) {
    Serial.println("🔄 Damper " + String(damperIndex) + ": Toggling to position " + String(currentPositionIndex) + " (" + String(newAngle) + "°)");
  }
  
  setAngle(newAngle);
}

void DamperController::processButton() {
  unsigned long currentTime = millis();
  bool rawButtonReading = (digitalRead(buttonPin) == LOW);
  
  // Debounce logic
  if (rawButtonReading != lastButtonState) {
    lastDebounceTime = currentTime;
  }
  
  if (currentTime - lastDebounceTime > DEBOUNCE_DELAY) {
    debouncedButtonState = rawButtonReading;
  }
  
  lastButtonState = rawButtonReading;
  
  // Detect button press (transition from HIGH to LOW)
  static bool lastDebouncedState = false;
  if (debouncedButtonState && !lastDebouncedState) {
    // Button pressed
    togglePosition();
  }
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
  static int lastPublishedAngle = -1;
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

