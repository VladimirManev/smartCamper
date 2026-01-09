// Table Controller
// Controls table lift with two relays (up/down) and buttons
// Supports manual hold-to-move and auto 5-second movement on double-click

#ifndef TABLE_CONTROLLER_H
#define TABLE_CONTROLLER_H

#include "Config.h"
#include <Arduino.h>

// Forward declaration
class MQTTManager;

class TableController {
private:
  int relayUpPin;
  int relayDownPin;
  int buttonUpPin;
  int buttonDownPin;
  MQTTManager* mqttManager;
  
  // Relay state
  bool relayUpActive;
  bool relayDownActive;
  
  // Button state
  bool lastButtonUpState;
  bool lastButtonDownState;
  bool debouncedButtonUpState;
  bool debouncedButtonDownState;
  unsigned long lastDebounceUpTime;
  unsigned long lastDebounceDownTime;
  static const unsigned long DEBOUNCE_DELAY = 50;  // ms
  
  // Double-click detection
  unsigned long lastClickUpTime;
  unsigned long lastClickDownTime;
  bool waitingForDoubleClickUp;
  bool waitingForDoubleClickDown;
  
  // Auto movement (5-second timer)
  bool autoMoving;
  unsigned long autoMoveStartTime;
  bool autoMoveDirection;  // true = up, false = down
  
  // Internal methods
  void processButtons();
  void updateRelays();
  void publishStatus();
  
  // Helper methods
  bool isMoving() const { return relayUpActive || relayDownActive; }
  String getDirection() const;
  void stopMovement();

public:
  TableController(int relayUp, int relayDown, int btnUp, int btnDown, MQTTManager* mqtt);
  
  // Initialization
  void begin();
  
  // Control
  void moveUp();           // Start moving up (manual hold)
  void moveDown();         // Start moving down (manual hold)
  void stop();             // Stop movement
  void moveUpAuto(int durationMs = TABLE_AUTO_MOVE_DURATION);   // Auto move up for duration
  void moveDownAuto(int durationMs = TABLE_AUTO_MOVE_DURATION); // Auto move down for duration
  
  // Status
  bool isMovingUp() const { return relayUpActive; }
  bool isMovingDown() const { return relayDownActive; }
  
  // Main loop - call this in your main loop()
  void loop();
  
  // Force update - publish current status
  void forceUpdate();
  
  // Status
  void printStatus() const;
};

#endif

