// Damper Controller
// Controls a single damper (air vent) with servo motor and button
// Supports 3 positions: 0°, 45°, 90°

#ifndef DAMPER_CONTROLLER_H
#define DAMPER_CONTROLLER_H

#include "Config.h"
#include "ServoController.h"

// Forward declaration
class MQTTManager;

class DamperController {
private:
  ServoController servo;
  int buttonPin;
  int damperIndex;
  MQTTManager* mqttManager;
  
  // Button state
  bool lastButtonState;
  bool debouncedButtonState;
  bool lastDebouncedState;  // Previous debounced state for edge detection
  unsigned long lastDebounceTime;
  static const unsigned long DEBOUNCE_DELAY = 100;  // ms (increased for better debouncing)
  
  // Position management
  static const int NUM_POSITIONS = 3;
  static const int POSITIONS[NUM_POSITIONS];  // {0, 45, 90}
  int currentPositionIndex;  // 0, 1, or 2
  
  // Internal methods
  void processButton();
  void togglePosition();
  void publishStatus();
  int getCurrentAngle() const;

public:
  DamperController(int index, int servoPin, int btnPin, MQTTManager* mqtt);
  
  // Initialization
  void begin();
  
  // Control
  void setAngle(int angle);  // Set specific angle (0-180°)
  int getAngle() const { return getCurrentAngle(); }
  
  // Main loop - call this in your main loop()
  void loop();
  
  // Force update - publish current status
  void forceUpdate();
  
  // Status
  void printStatus() const;
};

#endif

