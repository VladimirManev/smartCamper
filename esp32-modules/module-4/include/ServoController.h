// Servo Controller
// Reusable servo motor control class
// Supports smooth movement and angle validation

#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H

#include <Arduino.h>
#include "Config.h"

// Forward declaration
class Servo;

class ServoController {
private:
  int servoPin;
  int currentAngle;
  int targetAngle;
  bool isAttached;
  bool isMoving;
  unsigned long lastMoveStep;
  Servo* servo;  // Servo object (forward declared)
  
  // Smooth movement settings
  static const int STEP_SIZE = 10;  // Degrees per step (increased for faster movement)
  static const int MOVE_DELAY = 20;  // ms between steps (reduced for maximum speed)
  
  // Internal methods
  void moveStep();

public:
  ServoController(int pin);
  ~ServoController();
  
  // Initialization
  void begin();
  void end();
  
  // Control
  void setAngle(int angle);  // Set target angle (0-180°), validates and moves smoothly
  int getCurrentAngle() const { return currentAngle; }
  int getTargetAngle() const { return targetAngle; }
  bool isAtTarget() const { return currentAngle == targetAngle; }
  bool isMovingToTarget() const { return isMoving; }
  
  // Main loop - call this in your main loop() for smooth movement
  void loop();
  
  // Status
  void printStatus() const;
};

#endif

