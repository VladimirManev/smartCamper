// StripState structure definition
// Shared between main.cpp and LEDControllerManager

#ifndef STRIP_STATE_H
#define STRIP_STATE_H

#include <Arduino.h>

// User-facing strip mode (all strips; only bathroom uses AUTO / PIR)
enum StripMode {
  STRIP_MODE_OFF,   // 0: off
  STRIP_MODE_AUTO,  // 1: motion (strip 3 only; ignored on other strips)
  STRIP_MODE_ON     // 2: on
};

// Visual effect when output is on (solid channels vs patterns / animations)
enum StripEffect {
  STRIP_EFFECT_NORMAL = 0,
  STRIP_EFFECT_RAINBOW_STATIC = 1,
  STRIP_EFFECT_FIREWORKS = 2,  // Classic WLED-style fireworks (animated)
};

// Transition types (no random-LED wipe — removed)
enum TransitionType {
  TRANSITION_NONE = 0,
  TRANSITION_ON_CENTER_TO_EDGES,
  TRANSITION_ON_LEFT_TO_RIGHT,
  TRANSITION_ON_EDGES_TO_CENTER,
  TRANSITION_OFF_EDGES_TO_CENTER,
  TRANSITION_OFF_LEFT_TO_RIGHT,
  TRANSITION_OFF_CENTER_TO_EDGES
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
  
  // RGBW mix at "full" recipe; scaled by brightness in firmware
  uint8_t chR;
  uint8_t chG;
  uint8_t chB;
  uint8_t chW;
  StripEffect effect;
  unsigned long effectFrameTime;  // Last fireworks (etc.) frame time
  
  StripMode mode;
  uint8_t lastAutoBrightness;  // AUTO (strip 3): level for next PIR on
  
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

