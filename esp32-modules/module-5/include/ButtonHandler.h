// Button Handler
// Handles button input processing (debouncing, state machine)

#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include "Config.h"
#include <Arduino.h>

// Forward declarations
class RelayController;
class ApplianceManager;

// Button states
enum ButtonState {
  BUTTON_IDLE,
  BUTTON_PRESSED
};

class ButtonHandler {
private:
  RelayController* relayController;
  ApplianceManager* applianceManager;  // For publishing status updates
  
  struct ButtonStateMachine {
    ButtonState state;
    unsigned long pressTime;
    uint8_t pin;
    uint8_t relayIndex;  // Which relay this button controls
    
    // Debounce state
    bool lastRawReading;
    unsigned long lastDebounceTime;
    bool debouncedState;
  };
  
  ButtonStateMachine buttons[NUM_BUTTONS];
  
  // Button configuration - maps button index to relay index
  uint8_t buttonRelayMap[NUM_BUTTONS] = {0, 1, 2, 3, 4, 5};  // Button 0 -> Relay 0, Button 1 -> Relay 1, Button 2 -> Relay 2, Button 3 -> Relay 3, Button 4 -> Relay 4, Button 5 -> Relay 5
  
  void processButton(uint8_t btnIndex, unsigned long currentTime);
  
public:
  ButtonHandler(RelayController* relayCtrl);
  
  // Set ApplianceManager reference (for status publishing)
  void setApplianceManager(ApplianceManager* appMgr);
  
  // Initialization
  void begin();
  
  // Main loop - call this in your main loop()
  void loop();
  
  // Check if any button is currently pressed
  bool isAnyButtonPressed() const;
  
  // Status
  void printStatus() const;
};

#endif
