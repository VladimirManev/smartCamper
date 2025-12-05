// StripState structure definition
// Shared between main.cpp and LEDControllerManager

#ifndef STRIP_STATE_H
#define STRIP_STATE_H

#include <Arduino.h>

// Strip mode (for motion-activated strips)
enum StripMode {
  STRIP_MODE_OFF,   // 0: Strip is off
  STRIP_MODE_AUTO,  // 1: Automatic (motion-activated)
  STRIP_MODE_ON     // 2: Strip is on
};

// Transition types
enum TransitionType {
  TRANSITION_NONE,
  TRANSITION_ON_CENTER_TO_EDGES,      // 0: Smoothly from center to edges
  TRANSITION_ON_RANDOM_LEDS,           // 1: Random LEDs sequentially
  TRANSITION_ON_LEFT_TO_RIGHT,         // 2: From left to right
  TRANSITION_ON_EDGES_TO_CENTER,       // 3: From edges to center
  TRANSITION_OFF_EDGES_TO_CENTER,      // 4: From edges to center
  TRANSITION_OFF_RANDOM_LEDS,          // 5: Random LEDs sequentially
  TRANSITION_OFF_LEFT_TO_RIGHT,        // 6: From left to right
  TRANSITION_OFF_CENTER_TO_EDGES       // 7: From center to edges
};

// Transition state
struct TransitionState {
  bool active;
  TransitionType type;
  unsigned long startTime;
  uint8_t targetBrightness;
  uint8_t* randomOrder;
  int randomIndex;
};

// Strip state - using void* to support different types
struct StripState {
  void* strip;  // Pointer to LedStrip0, LedStrip1, LedStrip2, or LedStrip3
  uint8_t stripType;  // 0 = LedStrip0 (RMT0), 1 = LedStrip1 (RMT1), 2 = LedStrip2 (RMT2), 3 = LedStrip3 (RMT3)
  bool on;
  uint8_t brightness;
  
  // Mode (for motion-activated strips like Strip 3)
  StripMode mode;  // OFF, AUTO, or ON
  uint8_t lastAutoBrightness;  // Remember brightness when in AUTO mode
  
  // Dimming
  bool dimmingActive;
  bool dimmingDirection;  // true = increase, false = decrease
  unsigned long dimmingStartTime;
  uint8_t dimmingStartBrightness;
  uint8_t dimmingTargetBrightness;  // Target brightness for smooth transitions (MQTT commands)
  unsigned long dimmingDuration;  // dimming time in milliseconds (calculated dynamically)
  bool lastDimmingWasIncrease;
  bool isSmoothTransition;  // true = smooth transition to specific value (MQTT), false = dim to min/max (button)
  
  // Blinking
  bool blinkActive;
  unsigned long blinkStartTime;
  uint8_t savedBrightnessForBlink;
  
  // Transitions
  TransitionState transition;
};

#endif

