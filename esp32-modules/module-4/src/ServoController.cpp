// Servo Controller Implementation
// Reusable servo motor control class

#include "ServoController.h"
#include <ESP32Servo.h>

ServoController::ServoController(int pin) 
  : servoPin(pin), currentAngle(0), targetAngle(0), isAttached(false), isMoving(false), lastMoveStep(0), servo(nullptr) {
  servo = new Servo();
}

ServoController::~ServoController() {
  if (servo) {
    if (isAttached) {
      servo->detach();
    }
    delete servo;
  }
}

void ServoController::begin() {
  if (isAttached) {
    return;  // Already initialized
  }
  
  if (servo) {
    servo->attach(servoPin);
    servo->write(0);  // Start at 0 degrees
  }
  currentAngle = 0;
  targetAngle = 0;
  isAttached = true;
  isMoving = false;
  
  if (DEBUG_SERIAL) {
    Serial.println("🔧 ServoController initialized on pin " + String(servoPin));
  }
}

void ServoController::end() {
  if (!isAttached || !servo) {
    return;
  }
  
  servo->detach();
  isAttached = false;
  isMoving = false;
  
  if (DEBUG_SERIAL) {
    Serial.println("🔧 ServoController detached from pin " + String(servoPin));
  }
}

void ServoController::setAngle(int angle) {
  // Validate angle (0-180°)
  if (angle < 0 || angle > 180) {
    // Invalid angle - ignore silently
    return;
  }
  
  targetAngle = angle;
  
  // If already at target, no movement needed
  if (currentAngle == targetAngle) {
    isMoving = false;
    return;
  }
  
  // Start smooth movement
  isMoving = true;
  lastMoveStep = millis();
  
  if (DEBUG_SERIAL) {
    Serial.println("🔄 ServoController: Moving from " + String(currentAngle) + "° to " + String(targetAngle) + "°");
  }
}

void ServoController::loop() {
  if (!isAttached || !isMoving) {
    return;  // Not initialized or not moving
  }
  
  unsigned long currentTime = millis();
  
  // Check if it's time to move
  if (currentTime - lastMoveStep >= MOVE_DELAY) {
    moveStep();
    lastMoveStep = currentTime;
  }
}

void ServoController::moveStep() {
  if (currentAngle < targetAngle) {
    currentAngle += STEP_SIZE;
    if (currentAngle > targetAngle) currentAngle = targetAngle;
  } else if (currentAngle > targetAngle) {
    currentAngle -= STEP_SIZE;
    if (currentAngle < targetAngle) currentAngle = targetAngle;
  }
  
  // Write to servo
  if (servo && isAttached) {
    servo->write(currentAngle);
  }
  
  // Check if reached target
  if (currentAngle == targetAngle) {
    isMoving = false;
    if (DEBUG_SERIAL) {
      Serial.println("✅ ServoController: Reached target " + String(targetAngle) + "°");
    }
  }
}

void ServoController::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("📊 ServoController Status:");
    Serial.println("  Pin: " + String(servoPin));
    Serial.println("  Current Angle: " + String(currentAngle) + "°");
    Serial.println("  Target Angle: " + String(targetAngle) + "°");
    Serial.println("  Is Moving: " + String(isMoving ? "Yes" : "No"));
    Serial.println("  Is Attached: " + String(isAttached ? "Yes" : "No"));
  }
}

