// Button Handler
// Handles button input processing (debouncing, state machine, hold detection)

#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include "Config.h"
#include <Arduino.h>

// Forward declarations
class LEDStripController;
class RelayController;
class LEDManager;

// Button states
enum ButtonState {
  BUTTON_IDLE,
  BUTTON_PRESSED,
  BUTTON_HELD
};

// Button configuration
struct ButtonConfig {
  uint8_t pin;
  uint8_t stripIndex;  // Which strip this button controls (255 = relay, not a strip)
};

class ButtonHandler {
private:
  LEDStripController* ledController;
  RelayController* relayController;
  LEDManager* ledManager;  // For publishing status updates
  
  struct ButtonStateMachine {
    ButtonState state;
    unsigned long pressTime;
    uint8_t pin;
    uint8_t stripIndex;
    
    // Debounce state
    bool lastRawReading;
    unsigned long lastDebounceTime;
    bool debouncedState;
  };
  
  ButtonStateMachine buttons[NUM_BUTTONS];
  
  // Button configuration
  ButtonConfig buttonConfigs[NUM_BUTTONS] = {
    {BUTTON_PIN_1, 0},   // Button 0 -> Strip 0 (Kitchen - controls Strip 0 and Strip 2)
    {BUTTON_PIN_2, 1},   // Button 1 -> Strip 1
    {BUTTON_PIN_3, 255}, // Button 2 -> Relay (stripIndex 255 = relay, not a strip)
    {BUTTON_PIN_4, 4}    // Button 3 -> Strip 4 (Bedroom)
  };
  
  void processButton(uint8_t btnIndex, unsigned long currentTime);
  
public:
  ButtonHandler(LEDStripController* ledCtrl, RelayController* relayCtrl);
  
  // Set LEDManager reference (for status publishing)
  void setLEDManager(LEDManager* ledMgr);
  
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

