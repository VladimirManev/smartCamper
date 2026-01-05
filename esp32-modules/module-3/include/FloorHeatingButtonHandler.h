// Floor Heating Button Handler
// Handles button input for manual control of heating circles

#ifndef FLOOR_HEATING_BUTTON_HANDLER_H
#define FLOOR_HEATING_BUTTON_HANDLER_H

#include "Config.h"
#include <Arduino.h>

// Forward declarations
class FloorHeatingController;
class FloorHeatingManager;

class FloorHeatingButtonHandler {
private:
  struct ButtonState {
    uint8_t pin;
    uint8_t circleIndex;
    bool lastRawReading;
    unsigned long lastDebounceTime;
    bool debouncedState;
    bool lastPressedState;  // Last stable pressed state (to detect toggle)
  };
  
  ButtonState buttons[NUM_HEATING_CIRCLES];
  FloorHeatingController* controller;
  FloorHeatingManager* manager;  // For status publishing
  
  void processButton(uint8_t btnIndex, unsigned long currentTime);
  
public:
  FloorHeatingButtonHandler(FloorHeatingController* ctrl);
  
  // Initialization
  void begin();
  
  // Main loop - call this in your main loop()
  void loop();
  
  // Set controller reference
  void setController(FloorHeatingController* ctrl);
  
  // Set manager reference (for status publishing)
  void setManager(FloorHeatingManager* mgr);
  
  void printStatus() const;
};

#endif

