// LED Controller - Multi-strip support with abstraction

#include <Arduino.h>
#include <NeoPixelBus.h>
#include "LEDControllerManager.h"

// ============================================================================
// CONFIGURATION
// ============================================================================

// Number of strips (easily changeable)
#define NUM_STRIPS 5

// Strip settings (add new strips here)
struct StripConfig {
  uint8_t pin;
  uint16_t ledCount;
};

StripConfig stripConfigs[NUM_STRIPS] = {
  {33, 44},   // Strip 0: Pin 33, 44 LEDs - Kitchen (main)
  {18, 178},  // Strip 1: Pin 18, 178 LEDs - Main lighting
  {19, 23},   // Strip 2: Pin 19, 23 LEDs - Kitchen (extension for spice rack, mirrors strip 0)
  {25, 53},   // Strip 3: Pin 25, 53 LEDs - Bathroom (motion activated, no button, no dimming)
  {5, 60}     // Strip 4: Pin 5, 60 LEDs - Bedroom
};

// Button settings
#define NUM_BUTTONS 4    // Number of buttons (Strip 2 is automatically controlled by Strip 0)
#define BUTTON_PIN_1 4   // Button for strip 0 (Kitchen - controls Strip 0 and Strip 2)
#define BUTTON_PIN_2 12  // Button for strip 1
#define BUTTON_PIN_3 27  // Button for relay circuit (toggle button)
#define BUTTON_PIN_4 13  // Button for strip 4 (Bedroom)

// Relay settings (for LED diodes circuit)
// NUM_RELAYS е дефинирана в Config.h
#define RELAY_PIN_0 26   // Pin for relay 0 control (OUTPUT)

// PIR sensor settings (HC-SR501)
#define PIR_SENSOR_PIN 2        // Pin for PIR sensor
#define PIR_MOTION_TIMEOUT 60000  // 60 seconds (1 minute) after last motion detected
#define MOTION_STRIP_INDEX 3    // Strip 3 (Bathroom) is controlled by the sensor

// Brightness settings
#define MIN_BRIGHTNESS 1
#define MAX_BRIGHTNESS 255
#define DEFAULT_BRIGHTNESS 128  // 50% on first power on

// Dimming settings
#define DIMMING_SPEED 50  // brightness units per second (change speed)
#define HOLD_THRESHOLD 250  // 250ms before dimming starts
#define BLINK_DURATION 300  // Blink duration at max brightness (ms)
#define BLINK_MIN_FACTOR 0.3  // Minimum brightness during blink (30% of current)

// Transition settings
#define TRANSITION_DURATION 1000  // 1 second for transitions
#define NUM_ON_TRANSITIONS 4   // Number of transitions for turning on
#define NUM_OFF_TRANSITIONS 4  // Number of transitions for turning off

// ============================================================================
// STRUCTURES AND TYPES
// ============================================================================

// Strip type: WS2815 RGBW
// For ESP32 we use RMT methods - each strip must use a different RMT channel
// Strip 0 and Strip 2 (Kitchen) are synchronized in software, but use different RMT channels
typedef NeoPixelBus<NeoRgbwFeature, NeoEsp32Rmt0Ws2812xMethod> LedStrip0;
typedef NeoPixelBus<NeoRgbwFeature, NeoEsp32Rmt1Ws2812xMethod> LedStrip1;
typedef NeoPixelBus<NeoRgbwFeature, NeoEsp32Rmt2Ws2812xMethod> LedStrip2;
typedef NeoPixelBus<NeoRgbwFeature, NeoEsp32Rmt3Ws2812xMethod> LedStrip3;
// Bedroom strip - RGB type (no white channel)
// Using WS2812x protocol (most common) with RMT4 channel
typedef NeoPixelBus<NeoRgbFeature, NeoEsp32Rmt4Ws2812xMethod> LedStrip4;

// Common type for pointers
typedef NeoPixelBus<NeoRgbwFeature, NeoEsp32Rmt0Ws2812xMethod> LedStrip;

// Button states
enum ButtonState {
  BUTTON_IDLE,
  BUTTON_PRESSED,
  BUTTON_HELD
};

// Include StripState definition from header (includes TransitionType, TransitionState, and StripState)
#include "StripState.h"

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

// Array of strips - using static objects with different RMT channels
LedStrip0 strip0(stripConfigs[0].ledCount, stripConfigs[0].pin);  // Kitchen main
LedStrip1 strip1(stripConfigs[1].ledCount, stripConfigs[1].pin);  // Main lighting
LedStrip2 strip2(stripConfigs[2].ledCount, stripConfigs[2].pin);  // Kitchen extension (spice rack)
LedStrip3 strip3(stripConfigs[3].ledCount, stripConfigs[3].pin);  // Bathroom (motion activated)
LedStrip4 strip4(stripConfigs[4].ledCount, stripConfigs[4].pin);  // Bedroom

// Pointers to strips (for universality)
LedStrip* strips[NUM_STRIPS] = {(LedStrip*)&strip0, (LedStrip*)&strip1, (LedStrip*)&strip2, (LedStrip*)&strip3, (LedStrip*)&strip4};
StripState stripStates[NUM_STRIPS];

// Buttons
struct ButtonStateMachine {
  ButtonState state;
  unsigned long pressTime;
  uint8_t pin;
  uint8_t stripIndex;  // Which strip this button controls
  
  // Debounce state
  bool lastRawReading;
  unsigned long lastDebounceTime;
  bool debouncedState;
};

ButtonStateMachine buttons[NUM_BUTTONS] = {
  {BUTTON_IDLE, 0, BUTTON_PIN_1, 0, false, 0, false},  // Button 0 -> Strip 0 (Kitchen - controls Strip 0 and Strip 2)
  {BUTTON_IDLE, 0, BUTTON_PIN_2, 1, false, 0, false},  // Button 1 -> Strip 1
  {BUTTON_IDLE, 0, BUTTON_PIN_3, 255, false, 0, false}, // Button 2 -> Relay (stripIndex 255 = relay, not a strip)
  {BUTTON_IDLE, 0, BUTTON_PIN_4, 4, false, 0, false}   // Button 3 -> Strip 4 (Bedroom)
};

// Relay states (array for multiple relays)
bool relayStates[NUM_RELAYS] = {false};  // false = OFF, true = ON

// LED Controller Manager (WiFi + MQTT)
LEDControllerManager ledControllerManager;

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

// R and G are swapped in hardware
RgbwColor fixColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  return RgbwColor(g, r, b, w);
}

// Helper function to create white color using RGB and W channels
RgbwColor whiteColor(uint8_t brightness) {
  // Use RGB channels for white color in addition to W channel
  return RgbwColor(brightness, brightness, brightness, brightness);
}

// Helper function to create white color for RGB strips (no W channel)
RgbColor whiteColorRgb(uint8_t brightness) {
  // Use RGB channels for white color
  return RgbColor(brightness, brightness, brightness);
}

// Helper function to create warm white color for RGB strips (Bedroom - Strip 4)
// Warm white: more red, less blue for cozy bedroom lighting
RgbwColor warmWhiteColor(uint8_t brightness) {
  // Warm white proportions: R=100%, G=90%, B=75%
  // This creates a warmer, more cozy light suitable for bedroom
  uint8_t r = brightness;                    // 100% red
  uint8_t g = (brightness * 90) / 100;      // 90% green
  uint8_t b = (brightness * 75) / 100;      // 75% blue
  return RgbwColor(r, g, b, 0);  // No W channel for RGB strip
}

// Helper function to get the appropriate white color for a strip
// Strip 4 (Bedroom) uses warm white, others use standard white
RgbwColor getWhiteColorForStrip(uint8_t stripIndex, uint8_t brightness) {
  if (stripIndex == 4) {
    return warmWhiteColor(brightness);
  } else {
    return whiteColor(brightness);
  }
}

// Helper functions for working with different strip types
void setPixelColor(uint8_t stripIndex, int pixelIndex, RgbwColor color) {
  if (stripIndex >= NUM_STRIPS) return;
  StripState& state = stripStates[stripIndex];
  if (state.stripType == 0) {
    ((LedStrip0*)state.strip)->SetPixelColor(pixelIndex, color);
  } else if (state.stripType == 1) {
    ((LedStrip1*)state.strip)->SetPixelColor(pixelIndex, color);
  } else if (state.stripType == 2) {
    ((LedStrip2*)state.strip)->SetPixelColor(pixelIndex, color);
  } else if (state.stripType == 3) {
    ((LedStrip3*)state.strip)->SetPixelColor(pixelIndex, color);
  } else if (state.stripType == 4) {
    // Strip 4 is RGB, convert RgbwColor to RgbColor
    RgbColor rgbColor(color.R, color.G, color.B);
    ((LedStrip4*)state.strip)->SetPixelColor(pixelIndex, rgbColor);
  }
}

void clearStrip(uint8_t stripIndex, RgbwColor color) {
  if (stripIndex >= NUM_STRIPS) return;
  StripState& state = stripStates[stripIndex];
  if (state.stripType == 0) {
    ((LedStrip0*)state.strip)->ClearTo(color);
  } else if (state.stripType == 1) {
    ((LedStrip1*)state.strip)->ClearTo(color);
  } else if (state.stripType == 2) {
    ((LedStrip2*)state.strip)->ClearTo(color);
  } else if (state.stripType == 3) {
    ((LedStrip3*)state.strip)->ClearTo(color);
  } else if (state.stripType == 4) {
    // Strip 4 is RGB, convert RgbwColor to RgbColor
    RgbColor rgbColor(color.R, color.G, color.B);
    ((LedStrip4*)state.strip)->ClearTo(rgbColor);
  }
}

void showStrip(uint8_t stripIndex) {
  if (stripIndex >= NUM_STRIPS) return;
  StripState& state = stripStates[stripIndex];
  if (state.stripType == 0) {
    ((LedStrip0*)state.strip)->Show();
  } else if (state.stripType == 1) {
    ((LedStrip1*)state.strip)->Show();
  } else if (state.stripType == 2) {
    ((LedStrip2*)state.strip)->Show();
  } else if (state.stripType == 3) {
    ((LedStrip3*)state.strip)->Show();
  } else if (state.stripType == 4) {
    ((LedStrip4*)state.strip)->Show();
  }
}

// Macros for easier use in transitions
#define STRIP_CLEAR(idx, color) clearStrip(idx, color)
#define STRIP_SET_PIXEL(idx, pixel, color) setPixelColor(idx, pixel, color)
#define STRIP_SHOW(idx) showStrip(idx)

// Helper function for synchronizing Kitchen (Strip 0 and Strip 2)
void syncKitchenExtension(uint8_t mainStripIndex) {
  if (mainStripIndex == 0) {
    StripState& mainState = stripStates[0];
    StripState& extState = stripStates[2];
    
    // Copy state from main to extension
    extState.on = mainState.on;
    extState.brightness = mainState.brightness;
    extState.dimmingActive = mainState.dimmingActive;
    extState.dimmingDirection = mainState.dimmingDirection;
    extState.dimmingStartTime = mainState.dimmingStartTime;
    extState.dimmingStartBrightness = mainState.dimmingStartBrightness;
    extState.dimmingDuration = mainState.dimmingDuration;
    extState.blinkActive = mainState.blinkActive;
    extState.blinkStartTime = mainState.blinkStartTime;
    extState.savedBrightnessForBlink = mainState.savedBrightnessForBlink;
    extState.transition.active = mainState.transition.active;
    extState.transition.type = mainState.transition.type;
    extState.transition.startTime = mainState.transition.startTime;
    extState.transition.targetBrightness = mainState.transition.targetBrightness;
    
    // Update extension strip
    if (extState.on) {
      for (int i = 0; i < stripConfigs[2].ledCount; i++) {
        setPixelColor(2, i, whiteColor(extState.brightness));
      }
    } else {
      clearStrip(2, RgbwColor(0, 0, 0, 0));
    }
    showStrip(2);
  }
}

// Update strip with current brightness
void updateStrip(uint8_t stripIndex) {
  if (stripIndex >= NUM_STRIPS) return;
  
  StripState& state = stripStates[stripIndex];
  
  if (state.on) {
    for (int i = 0; i < stripConfigs[stripIndex].ledCount; i++) {
      setPixelColor(stripIndex, i, getWhiteColorForStrip(stripIndex, state.brightness));
    }
  } else {
    clearStrip(stripIndex, RgbwColor(0, 0, 0, 0));
  }
  showStrip(stripIndex);
  
  // Kitchen: synchronize extension strip
  syncKitchenExtension(stripIndex);
}

// ============================================================================
// TURN ON TRANSITIONS
// ============================================================================

void transitionOnCenterToEdges(uint8_t stripIndex) {
  StripState& state = stripStates[stripIndex];
  TransitionState& trans = state.transition;
  uint16_t ledCount = stripConfigs[stripIndex].ledCount;
  
  unsigned long elapsed = millis() - trans.startTime;
  float progress = (float)elapsed / TRANSITION_DURATION;
  if (progress > 1.0) progress = 1.0;
  
  int center = ledCount / 2;
  int maxDistance = center;
  int currentDistance = (int)(maxDistance * progress);
  
  STRIP_CLEAR(stripIndex, RgbwColor(0, 0, 0, 0));
  for (int i = 0; i <= currentDistance; i++) {
    if (center - i >= 0) {
      STRIP_SET_PIXEL(stripIndex, center - i, getWhiteColorForStrip(stripIndex, trans.targetBrightness));
    }
    if (center + i < ledCount) {
      STRIP_SET_PIXEL(stripIndex, center + i, getWhiteColorForStrip(stripIndex, trans.targetBrightness));
    }
  }
  STRIP_SHOW(stripIndex);
  
  if (progress >= 1.0) {
    trans.active = false;
  }
}

void transitionOnRandomLeds(uint8_t stripIndex) {
  StripState& state = stripStates[stripIndex];
  TransitionState& trans = state.transition;
  uint16_t ledCount = stripConfigs[stripIndex].ledCount;
  
  unsigned long elapsed = millis() - trans.startTime;
  float progress = (float)elapsed / TRANSITION_DURATION;
  if (progress > 1.0) progress = 1.0;
  
  if (trans.randomOrder == nullptr) {
    trans.randomOrder = new uint8_t[ledCount];
    for (int i = 0; i < ledCount; i++) {
      trans.randomOrder[i] = i;
    }
    for (int i = ledCount - 1; i > 0; i--) {
      int j = random(0, i + 1);
      uint8_t temp = trans.randomOrder[i];
      trans.randomOrder[i] = trans.randomOrder[j];
      trans.randomOrder[j] = temp;
    }
    trans.randomIndex = 0;
  }
  
  int targetCount = (int)(ledCount * progress);
  STRIP_CLEAR(stripIndex, RgbwColor(0, 0, 0, 0));
  
  for (int i = 0; i < targetCount && i < ledCount; i++) {
    STRIP_SET_PIXEL(stripIndex, trans.randomOrder[i], 
                    getWhiteColorForStrip(stripIndex, trans.targetBrightness));
  }
  STRIP_SHOW(stripIndex);
  
  if (progress >= 1.0) {
    delete[] trans.randomOrder;
    trans.randomOrder = nullptr;
    trans.active = false;
  }
}

void transitionOnLeftToRight(uint8_t stripIndex) {
  StripState& state = stripStates[stripIndex];
  TransitionState& trans = state.transition;
  uint16_t ledCount = stripConfigs[stripIndex].ledCount;
  
  unsigned long elapsed = millis() - trans.startTime;
  float progress = (float)elapsed / TRANSITION_DURATION;
  if (progress > 1.0) progress = 1.0;
  
  int currentEnd = (int)(ledCount * progress);
  
  STRIP_CLEAR(stripIndex, RgbwColor(0, 0, 0, 0));
  for (int i = 0; i < currentEnd; i++) {
    STRIP_SET_PIXEL(stripIndex, i, getWhiteColorForStrip(stripIndex, trans.targetBrightness));
  }
  STRIP_SHOW(stripIndex);
  
  if (progress >= 1.0) {
    trans.active = false;
  }
}

void transitionOnEdgesToCenter(uint8_t stripIndex) {
  StripState& state = stripStates[stripIndex];
  TransitionState& trans = state.transition;
  uint16_t ledCount = stripConfigs[stripIndex].ledCount;
  
  unsigned long elapsed = millis() - trans.startTime;
  float progress = (float)elapsed / TRANSITION_DURATION;
  if (progress > 1.0) progress = 1.0;
  
  int center = ledCount / 2;
  int maxDistance = center;
  int currentDistance = (int)(maxDistance * (1.0 - progress));
  
  STRIP_CLEAR(stripIndex, RgbwColor(0, 0, 0, 0));
  for (int i = 0; i <= maxDistance - currentDistance; i++) {
    if (center - i >= 0) {
      STRIP_SET_PIXEL(stripIndex, center - i, getWhiteColorForStrip(stripIndex, trans.targetBrightness));
    }
    if (center + i < ledCount) {
      STRIP_SET_PIXEL(stripIndex, center + i, getWhiteColorForStrip(stripIndex, trans.targetBrightness));
    }
  }
  STRIP_SHOW(stripIndex);
  
  if (progress >= 1.0) {
    trans.active = false;
  }
}

// ============================================================================
// TURN OFF TRANSITIONS
// ============================================================================

void transitionOffEdgesToCenter(uint8_t stripIndex) {
  StripState& state = stripStates[stripIndex];
  TransitionState& trans = state.transition;
  uint16_t ledCount = stripConfigs[stripIndex].ledCount;
  
  unsigned long elapsed = millis() - trans.startTime;
  float progress = (float)elapsed / TRANSITION_DURATION;
  if (progress > 1.0) progress = 1.0;
  
  int center = ledCount / 2;
  int maxDistance = center;
  int currentDistance = (int)(maxDistance * progress);
  
  for (int i = 0; i < ledCount; i++) {
    STRIP_SET_PIXEL(stripIndex, i, getWhiteColorForStrip(stripIndex, trans.targetBrightness));
  }
  
  for (int i = 0; i < currentDistance; i++) {
    if (i < ledCount) {
      STRIP_SET_PIXEL(stripIndex, i, RgbwColor(0, 0, 0, 0));
    }
    if (ledCount - 1 - i >= 0) {
      STRIP_SET_PIXEL(stripIndex, ledCount - 1 - i, RgbwColor(0, 0, 0, 0));
    }
  }
  
  // For odd number of LEDs, clear the middle LED when we reach the center
  if (ledCount % 2 == 1 && currentDistance >= center) {
    STRIP_SET_PIXEL(stripIndex, center, RgbwColor(0, 0, 0, 0));
  }
  
  STRIP_SHOW(stripIndex);
  
  if (progress >= 1.0) {
    trans.active = false;
  }
}

void transitionOffRandomLeds(uint8_t stripIndex) {
  StripState& state = stripStates[stripIndex];
  TransitionState& trans = state.transition;
  uint16_t ledCount = stripConfigs[stripIndex].ledCount;
  
  unsigned long elapsed = millis() - trans.startTime;
  float progress = (float)elapsed / TRANSITION_DURATION;
  if (progress > 1.0) progress = 1.0;
  
  if (trans.randomOrder == nullptr) {
    trans.randomOrder = new uint8_t[ledCount];
    for (int i = 0; i < ledCount; i++) {
      trans.randomOrder[i] = i;
    }
    for (int i = ledCount - 1; i > 0; i--) {
      int j = random(0, i + 1);
      uint8_t temp = trans.randomOrder[i];
      trans.randomOrder[i] = trans.randomOrder[j];
      trans.randomOrder[j] = temp;
    }
    trans.randomIndex = 0;
  }
  
  int offCount = (int)(ledCount * progress);
  
  for (int i = 0; i < ledCount; i++) {
    STRIP_SET_PIXEL(stripIndex, i, getWhiteColorForStrip(stripIndex, trans.targetBrightness));
  }
  
  for (int i = 0; i < offCount && i < ledCount; i++) {
    STRIP_SET_PIXEL(stripIndex, trans.randomOrder[i], RgbwColor(0, 0, 0, 0));
  }
  STRIP_SHOW(stripIndex);
  
  if (progress >= 1.0) {
    delete[] trans.randomOrder;
    trans.randomOrder = nullptr;
    trans.active = false;
  }
}

void transitionOffLeftToRight(uint8_t stripIndex) {
  StripState& state = stripStates[stripIndex];
  TransitionState& trans = state.transition;
  uint16_t ledCount = stripConfigs[stripIndex].ledCount;
  
  unsigned long elapsed = millis() - trans.startTime;
  float progress = (float)elapsed / TRANSITION_DURATION;
  if (progress > 1.0) progress = 1.0;
  
  int currentEnd = (int)(ledCount * progress);
  
  for (int i = 0; i < ledCount; i++) {
    STRIP_SET_PIXEL(stripIndex, i, getWhiteColorForStrip(stripIndex, trans.targetBrightness));
  }
  
  for (int i = 0; i < currentEnd; i++) {
    STRIP_SET_PIXEL(stripIndex, i, RgbwColor(0, 0, 0, 0));
  }
  STRIP_SHOW(stripIndex);
  
  if (progress >= 1.0) {
    trans.active = false;
  }
}

void transitionOffCenterToEdges(uint8_t stripIndex) {
  StripState& state = stripStates[stripIndex];
  TransitionState& trans = state.transition;
  uint16_t ledCount = stripConfigs[stripIndex].ledCount;
  
  unsigned long elapsed = millis() - trans.startTime;
  float progress = (float)elapsed / TRANSITION_DURATION;
  if (progress > 1.0) progress = 1.0;
  
  int center = ledCount / 2;
  int maxDistance = center;
  int currentDistance = (int)(maxDistance * progress);
  
  for (int i = 0; i < ledCount; i++) {
    STRIP_SET_PIXEL(stripIndex, i, getWhiteColorForStrip(stripIndex, trans.targetBrightness));
  }
  
  for (int i = 0; i <= currentDistance; i++) {
    if (center - i >= 0) {
      STRIP_SET_PIXEL(stripIndex, center - i, RgbwColor(0, 0, 0, 0));
    }
    if (center + i < ledCount) {
      STRIP_SET_PIXEL(stripIndex, center + i, RgbwColor(0, 0, 0, 0));
    }
  }
  STRIP_SHOW(stripIndex);
  
  if (progress >= 1.0) {
    trans.active = false;
  }
}

// ============================================================================
// TRANSITION MANAGEMENT
// ============================================================================

typedef void (*TransitionFunction)(uint8_t);

TransitionFunction onTransitions[NUM_ON_TRANSITIONS] = {
  transitionOnCenterToEdges,
  transitionOnRandomLeds,
  transitionOnLeftToRight,
  transitionOnEdgesToCenter
};

TransitionFunction offTransitions[NUM_OFF_TRANSITIONS] = {
  transitionOffEdgesToCenter,
  transitionOffRandomLeds,
  transitionOffLeftToRight,
  transitionOffCenterToEdges
};

void startTransition(uint8_t stripIndex, bool turningOn) {
  if (stripIndex >= NUM_STRIPS) return;
  
  StripState& state = stripStates[stripIndex];
  TransitionState& trans = state.transition;
  
  if (trans.active) return;
  
  trans.active = true;
  trans.startTime = millis();
  trans.targetBrightness = state.brightness;
  trans.randomOrder = nullptr;
  trans.randomIndex = 0;
  
  if (turningOn) {
    int index = random(0, NUM_ON_TRANSITIONS);
    trans.type = (TransitionType)index;
    Serial.println("✨ Strip " + String(stripIndex) + " ON transition " + String(index));
  } else {
    int index = random(0, NUM_OFF_TRANSITIONS);
    trans.type = (TransitionType)(NUM_ON_TRANSITIONS + index);
    Serial.println("✨ Strip " + String(stripIndex) + " OFF transition " + String(index));
  }
}

void updateTransition(uint8_t stripIndex) {
  if (stripIndex >= NUM_STRIPS) return;
  
  StripState& state = stripStates[stripIndex];
  TransitionState& trans = state.transition;
  
  if (!trans.active) return;
  
  // Debug - show we're working only with this strip
  static unsigned long lastDebugTime[NUM_STRIPS] = {0, 0};
  unsigned long now = millis();
  if (now - lastDebugTime[stripIndex] > 200) {
    lastDebugTime[stripIndex] = now;
    // Serial.println("Updating transition for strip " + String(stripIndex));
  }
  
  if (trans.type < NUM_ON_TRANSITIONS) {
    onTransitions[trans.type](stripIndex);
  } else {
    int offIndex = trans.type - NUM_ON_TRANSITIONS;
    offTransitions[offIndex](stripIndex);
  }
  
  // Kitchen: synchronize extension strip - execute the same transition
  if (stripIndex == 0 && stripStates[2].transition.active) {
    TransitionState& extTrans = stripStates[2].transition;
    if (extTrans.type < NUM_ON_TRANSITIONS) {
      onTransitions[extTrans.type](2);
    } else {
      int offIndex = extTrans.type - NUM_ON_TRANSITIONS;
      offTransitions[offIndex](2);
    }
  }
  
  if (!trans.active) {
    if (trans.type < NUM_ON_TRANSITIONS) {
      updateStrip(stripIndex);
      Serial.println("✅ Strip " + String(stripIndex) + " ON transition completed");
    } else {
      clearStrip(stripIndex, RgbwColor(0, 0, 0, 0));
      showStrip(stripIndex);
      Serial.println("✅ Strip " + String(stripIndex) + " OFF transition completed");
    }
  }
}

// ============================================================================
// BLINKING AT MIN/MAX
// ============================================================================

void updateBlink(uint8_t stripIndex) {
  if (stripIndex >= NUM_STRIPS) return;
  
  StripState& state = stripStates[stripIndex];
  
  if (!state.blinkActive || !state.on) return;
  
  unsigned long elapsed = millis() - state.blinkStartTime;
  
  if (elapsed < BLINK_DURATION) {
    float progress = (float)elapsed / BLINK_DURATION;
    float sineWave = sin(progress * PI);
    float brightnessFactor = 1.0 - (1.0 - BLINK_MIN_FACTOR) * sineWave;
    uint8_t currentBrightness = (uint8_t)(state.savedBrightnessForBlink * brightnessFactor);
    
    for (int i = 0; i < stripConfigs[stripIndex].ledCount; i++) {
      STRIP_SET_PIXEL(stripIndex, i, getWhiteColorForStrip(stripIndex, currentBrightness));
    }
    STRIP_SHOW(stripIndex);
    
    // Kitchen: synchronize extension strip - use the same brightnessFactor
    if (stripIndex == 0 && stripStates[2].blinkActive && stripStates[2].on) {
      StripState& extState = stripStates[2];
      uint8_t extBrightness = (uint8_t)(extState.savedBrightnessForBlink * brightnessFactor);
      for (int i = 0; i < stripConfigs[2].ledCount; i++) {
        STRIP_SET_PIXEL(2, i, whiteColor(extBrightness));
      }
      STRIP_SHOW(2);
    }
  } else {
    state.blinkActive = false;
    state.brightness = state.savedBrightnessForBlink;
    updateStrip(stripIndex);
  }
}

// ============================================================================
// DIMMING
// ============================================================================

void updateDimming(uint8_t stripIndex) {
  if (stripIndex >= NUM_STRIPS) return;
  
  StripState& state = stripStates[stripIndex];
  
  if (!state.dimmingActive || !state.on) return;
  
  unsigned long elapsed = millis() - state.dimmingStartTime;
  float progress = (float)elapsed / state.dimmingDuration;
  
  if (progress >= 1.0) {
    progress = 1.0;
    state.dimmingActive = false;
  }
  
  // Determine target brightness based on transition type
  uint8_t targetBrightness;
  if (state.isSmoothTransition) {
    // Smooth transition to specific value (MQTT command)
    targetBrightness = state.dimmingTargetBrightness;
  } else {
    // Dim to min/max (button hold)
    targetBrightness = state.dimmingDirection ? MAX_BRIGHTNESS : MIN_BRIGHTNESS;
  }
  
  uint8_t newBrightness = state.dimmingStartBrightness + (int)((targetBrightness - state.dimmingStartBrightness) * progress);
  
  if (newBrightness > MAX_BRIGHTNESS) newBrightness = MAX_BRIGHTNESS;
  if (newBrightness < MIN_BRIGHTNESS) newBrightness = MIN_BRIGHTNESS;
  
  bool reachedTarget = false;
  if (state.isSmoothTransition) {
    // For smooth transitions, check if we reached the target
    reachedTarget = (abs((int)newBrightness - (int)targetBrightness) <= 1) || (progress >= 1.0);
  } else {
    // For button dimming, check if we reached min/max
    if (state.dimmingDirection && newBrightness >= MAX_BRIGHTNESS) {
      newBrightness = MAX_BRIGHTNESS;
      reachedTarget = true;
    } else if (!state.dimmingDirection && newBrightness <= MIN_BRIGHTNESS) {
      newBrightness = MIN_BRIGHTNESS;
      reachedTarget = true;
    }
  }
  
  // Blinking only when reaching MAX via button dimming, not for smooth transitions
  if (reachedTarget && !state.blinkActive && !state.isSmoothTransition) {
    state.dimmingActive = false;
    state.lastDimmingWasIncrease = state.dimmingDirection;
    
    if (state.dimmingDirection) {
      // Only when increasing to MAX via button - blink
      state.blinkActive = true;
      state.blinkStartTime = millis();
      state.savedBrightnessForBlink = newBrightness;
      Serial.println("✨ Strip " + String(stripIndex) + " reached MAX brightness - blinking");
    } else {
      // When decreasing to MIN - no blinking
      Serial.println("✨ Strip " + String(stripIndex) + " reached MIN brightness");
    }
    
    state.brightness = newBrightness;
    syncKitchenExtension(stripIndex);
  } else if (reachedTarget && state.isSmoothTransition) {
    // Smooth transition completed
    state.dimmingActive = false;
    state.isSmoothTransition = false;
    state.brightness = targetBrightness;
    updateStrip(stripIndex);
    syncKitchenExtension(stripIndex);
    
    // Publish status after smooth transition completes
    ledControllerManager.publishStripStatus(stripIndex);
  } else {
    // Still transitioning
    state.brightness = newBrightness;
    updateStrip(stripIndex);
  }
}

// ============================================================================
// MAIN CONTROL FUNCTIONS (called from button, MQTT, sensors)
// ============================================================================

void turnOnStrip(uint8_t stripIndex) {
  if (stripIndex >= NUM_STRIPS) return;
  
  StripState& state = stripStates[stripIndex];
  if (state.on) {
    if (DEBUG_VERBOSE) {
      Serial.println("⚠️ turnOnStrip called for strip " + String(stripIndex) + " but it's already ON");
    }
    return;  // Already on
  }
  
  state.on = true;
  
  // Kitchen: if controlling Strip 0, synchronize Strip 2 BEFORE choosing transition
  // IMPORTANT: Only when stripIndex == 0, synchronize Strip 2 (pin 19)
  if (stripIndex == 0) {
    StripState& extState = stripStates[2];
    extState.on = true;
    extState.brightness = state.brightness;
    if (DEBUG_VERBOSE) {
      Serial.println("   Syncing Kitchen extension (Strip 2, pin " + String(stripConfigs[2].pin) + ")");
    }
  }
  
  // Choose transition once and use it for both strips (if Kitchen)
  startTransition(stripIndex, true);
  
  // Kitchen: copy the same transition to extension strip
  if (stripIndex == 0) {
    StripState& extState = stripStates[2];
    TransitionState& mainTrans = state.transition;
    TransitionState& extTrans = extState.transition;
    
    // Copy transition from main to extension
    extTrans.active = mainTrans.active;
    extTrans.type = mainTrans.type;
    extTrans.startTime = mainTrans.startTime;
    extTrans.targetBrightness = mainTrans.targetBrightness;
    extTrans.randomOrder = nullptr;  // Extension will use the same randomOrder if needed
    extTrans.randomIndex = 0;
    
    if (DEBUG_VERBOSE) {
      Serial.println("💡 Kitchen extension (Strip 2): Turning ON with same transition");
    }
  }
  
  Serial.println("💡 Strip " + String(stripIndex) + " ON (brightness: " + String(state.brightness) + ")");
  
  // Publish status after turning on
  ledControllerManager.publishStripStatus(stripIndex);
}

void turnOffStrip(uint8_t stripIndex) {
  if (stripIndex >= NUM_STRIPS) return;
  
  StripState& state = stripStates[stripIndex];
  if (!state.on) return;  // Already off
  
  // If in AUTO mode, remember brightness before turning off
  if (state.mode == STRIP_MODE_AUTO) {
    state.lastAutoBrightness = state.brightness;
  }
  
  state.on = false;
  
  // Kitchen: if controlling Strip 0, synchronize Strip 2 BEFORE choosing transition
  if (stripIndex == 0) {
    StripState& extState = stripStates[2];
    extState.on = false;
  }
  
  // Choose transition once and use it for both strips (if Kitchen)
  startTransition(stripIndex, false);
  
  // Kitchen: copy the same transition to extension strip
  if (stripIndex == 0) {
    StripState& extState = stripStates[2];
    TransitionState& mainTrans = state.transition;
    TransitionState& extTrans = extState.transition;
    
    // Copy transition from main to extension
    extTrans.active = mainTrans.active;
    extTrans.type = mainTrans.type;
    extTrans.startTime = mainTrans.startTime;
    extTrans.targetBrightness = mainTrans.targetBrightness;
    extTrans.randomOrder = nullptr;  // Extension will use the same randomOrder if needed
    extTrans.randomIndex = 0;
    
    if (DEBUG_VERBOSE) {
      Serial.println("💡 Kitchen extension (Strip 2): Turning OFF with same transition");
    }
  }
  
  Serial.println("💡 Strip " + String(stripIndex) + " OFF (brightness: " + String(state.brightness) + ")");
  
  // Publish status after turning off
  ledControllerManager.publishStripStatus(stripIndex);
}

void toggleStrip(uint8_t stripIndex) {
  if (stripIndex >= NUM_STRIPS) {
    Serial.println("ERROR: toggleStrip called with invalid stripIndex: " + String(stripIndex));
    return;
  }
  
  StripState& state = stripStates[stripIndex];
  Serial.println("🔄 toggleStrip(" + String(stripIndex) + ") - current state: " + String(state.on ? "ON" : "OFF"));
  Serial.println("   Strip 0 state: " + String(stripStates[0].on ? "ON" : "OFF") + ", Strip 1 state: " + String(stripStates[1].on ? "ON" : "OFF"));
  Serial.flush();
  
  if (state.on) {
    turnOffStrip(stripIndex);
  } else {
    turnOnStrip(stripIndex);
  }
  
  Serial.println("   After toggle - Strip 0 state: " + String(stripStates[0].on ? "ON" : "OFF") + ", Strip 1 state: " + String(stripStates[1].on ? "ON" : "OFF"));
  Serial.flush();
}

void toggleRelay(uint8_t relayIndex) {
  if (relayIndex >= NUM_RELAYS) return;
  
  relayStates[relayIndex] = !relayStates[relayIndex];
  uint8_t relayPin = (relayIndex == 0) ? RELAY_PIN_0 : 0;  // Add more pins as needed
  digitalWrite(relayPin, relayStates[relayIndex] ? HIGH : LOW);
  Serial.println("🔌 Relay " + String(relayIndex) + " " + String(relayStates[relayIndex] ? "ON" : "OFF") + " (Pin " + String(relayPin) + ")");
  
  // Publish status after relay toggle
  ledControllerManager.publishRelayStatus();
}

// Set strip mode (for motion-activated strips like Strip 3)
void setStripMode(uint8_t stripIndex, StripMode mode) {
  if (stripIndex >= NUM_STRIPS) return;
  
  StripState& state = stripStates[stripIndex];
  state.mode = mode;
  
  if (mode == STRIP_MODE_OFF) {
    // Turn off the strip
    if (state.on) {
      turnOffStrip(stripIndex);
    }
    Serial.println("🔧 Strip " + String(stripIndex) + " mode: OFF");
  } else if (mode == STRIP_MODE_ON) {
    // Turn on the strip with current brightness
    if (!state.on) {
      turnOnStrip(stripIndex);
    }
    Serial.println("🔧 Strip " + String(stripIndex) + " mode: ON");
  } else if (mode == STRIP_MODE_AUTO) {
    // AUTO mode - remember current brightness if strip is on
    if (state.on) {
      state.lastAutoBrightness = state.brightness;
    }
    // Turn off if currently on (will be controlled by PIR sensor)
    if (state.on) {
      turnOffStrip(stripIndex);
    }
    Serial.println("🔧 Strip " + String(stripIndex) + " mode: AUTO (brightness: " + String(state.lastAutoBrightness) + ")");
  }
  
  // Publish status after mode change
  ledControllerManager.publishStripStatus(stripIndex);
}

void startDimming(uint8_t stripIndex) {
  if (stripIndex >= NUM_STRIPS) return;
  
  // Strip 3 (motion activated) has no dimming
  if (stripIndex == MOTION_STRIP_INDEX) return;
  
  StripState& state = stripStates[stripIndex];
  if (!state.on || state.dimmingActive) return;
  
  state.dimmingActive = true;
  state.isSmoothTransition = false;  // Button dimming, not smooth transition
  state.dimmingStartTime = millis();
  state.dimmingStartBrightness = state.brightness;
  state.dimmingDirection = !state.lastDimmingWasIncrease;
  state.lastDimmingWasIncrease = state.dimmingDirection;
  
  // Calculate target brightness and time based on distance
  uint8_t targetBrightness = state.dimmingDirection ? MAX_BRIGHTNESS : MIN_BRIGHTNESS;
  state.dimmingTargetBrightness = targetBrightness;  // Set for consistency
  uint8_t distance = abs((int)targetBrightness - (int)state.dimmingStartBrightness);
  state.dimmingDuration = (distance * 1000) / DIMMING_SPEED;  // time in milliseconds
  
  Serial.println("🔆 Strip " + String(stripIndex) + " dimming: " + String(state.dimmingDirection ? "Increasing" : "Decreasing") + 
                 " (distance: " + String(distance) + ", time: " + String(state.dimmingDuration) + "ms)");
  
  // Kitchen: synchronize extension strip
  syncKitchenExtension(stripIndex);
}

void stopDimming(uint8_t stripIndex) {
  if (stripIndex >= NUM_STRIPS) return;
  
  StripState& state = stripStates[stripIndex];
  state.dimmingActive = false;
  Serial.println("🔆 Strip " + String(stripIndex) + " dimming stopped (Brightness: " + String(state.brightness) + ")");
  
  // Kitchen: synchronize extension strip
  syncKitchenExtension(stripIndex);
  
  // Publish status after dimming stops (for UI update)
  ledControllerManager.publishStripStatus(stripIndex);
}

// Set brightness smoothly (for MQTT commands)
// Duration: ~1 second for smooth transition
void setBrightnessSmooth(uint8_t stripIndex, uint8_t targetBrightness) {
  if (stripIndex >= NUM_STRIPS) return;
  
  StripState& state = stripStates[stripIndex];
  
  // Clamp target brightness
  if (targetBrightness < MIN_BRIGHTNESS) targetBrightness = MIN_BRIGHTNESS;
  if (targetBrightness > MAX_BRIGHTNESS) targetBrightness = MAX_BRIGHTNESS;
  
  // If strip is off, turn it on first (but don't show yet)
  uint8_t startBrightness = state.brightness;
  if (!state.on) {
    state.on = true;
    startBrightness = 0;  // Start from 0 for smooth fade-in
    
    // Kitchen: synchronize extension strip
    if (stripIndex == 0) {
      StripState& extState = stripStates[2];
      extState.on = true;
      extState.brightness = 0;
    }
  }
  
  // Stop any existing dimming
  state.dimmingActive = false;
  
  // Start smooth transition
  state.dimmingActive = true;
  state.isSmoothTransition = true;
  state.dimmingStartTime = millis();
  state.dimmingStartBrightness = startBrightness;
  state.dimmingTargetBrightness = targetBrightness;
  state.dimmingDirection = (targetBrightness > startBrightness);
  
  // Calculate duration: ~1 second for full range (255 units)
  uint8_t distance = abs((int)targetBrightness - (int)startBrightness);
  state.dimmingDuration = (distance * 1000) / 255;  // ~1 second for 255 units
  if (state.dimmingDuration < 200) state.dimmingDuration = 200;  // Minimum 200ms
  if (state.dimmingDuration > 2000) state.dimmingDuration = 2000;  // Maximum 2 seconds
  
  Serial.println("🔆 Strip " + String(stripIndex) + " smooth brightness change: " + 
                 String(startBrightness) + " → " + String(targetBrightness) + 
                 " (duration: " + String(state.dimmingDuration) + "ms)");
  
  // Kitchen: synchronize extension strip
  if (stripIndex == 0) {
    StripState& extState = stripStates[2];
    extState.dimmingActive = true;
    extState.isSmoothTransition = true;
    extState.dimmingStartTime = state.dimmingStartTime;
    extState.dimmingStartBrightness = 0;
    extState.dimmingTargetBrightness = targetBrightness;
    extState.dimmingDirection = state.dimmingDirection;
    extState.dimmingDuration = state.dimmingDuration;
  }
}

// Check if any button is currently pressed (for MQTT command blocking)
bool isAnyButtonPressed() {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    ButtonStateMachine& btn = buttons[i];
    // Check if button is in PRESSED or HELD state
    if (btn.state == BUTTON_PRESSED || btn.state == BUTTON_HELD) {
      return true;
    }
  }
  return false;
}

// ============================================================================
// SETUP AND LOOP
// ============================================================================

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n\n✨ LED Controller Starting...");
  Serial.println("Number of strips: " + String(NUM_STRIPS));
  
  // Initialize strips - using different RMT channels for each strip
  Serial.println("Initializing strip 0 on pin " + String(stripConfigs[0].pin) + " with RMT0...");
  Serial.flush();
  strip0.Begin();
  delay(100);
  strip0.ClearTo(RgbwColor(0, 0, 0, 0));
  strip0.Show();
  stripStates[0].strip = (void*)&strip0;
  stripStates[0].stripType = 0;  // LedStrip0 (RMT0)
  stripStates[0].on = false;
  stripStates[0].brightness = DEFAULT_BRIGHTNESS;
  stripStates[0].dimmingActive = false;
  stripStates[0].dimmingDirection = true;
  stripStates[0].dimmingTargetBrightness = DEFAULT_BRIGHTNESS;
  stripStates[0].isSmoothTransition = false;
  stripStates[0].lastDimmingWasIncrease = true;
  stripStates[0].blinkActive = false;
  stripStates[0].transition.active = false;
  stripStates[0].transition.randomOrder = nullptr;
  Serial.println("Strip 0 - Pin: " + String(stripConfigs[0].pin) + ", LEDs: " + String(stripConfigs[0].ledCount) + " - OK (RMT0)");
  
  Serial.println("Initializing strip 1 on pin " + String(stripConfigs[1].pin) + " with RMT1...");
  Serial.flush();
  strip1.Begin();
  delay(100);
  strip1.ClearTo(RgbwColor(0, 0, 0, 0));
  strip1.Show();
  stripStates[1].strip = (void*)&strip1;
  stripStates[1].stripType = 1;  // LedStrip1 (RMT1)
  stripStates[1].on = false;
  stripStates[1].brightness = DEFAULT_BRIGHTNESS;
  stripStates[1].dimmingActive = false;
  stripStates[1].dimmingDirection = true;
  stripStates[1].dimmingTargetBrightness = DEFAULT_BRIGHTNESS;
  stripStates[1].isSmoothTransition = false;
  stripStates[1].lastDimmingWasIncrease = true;
  stripStates[1].blinkActive = false;
  stripStates[1].transition.active = false;
  stripStates[1].transition.randomOrder = nullptr;
  Serial.println("Strip 1 - Pin: " + String(stripConfigs[1].pin) + ", LEDs: " + String(stripConfigs[1].ledCount) + " - OK (RMT1) Main lighting");
  
  Serial.println("Initializing strip 2 on pin " + String(stripConfigs[2].pin) + " with RMT2 (Kitchen extension - spice rack, synced with Strip 0)...");
  Serial.flush();
  strip2.Begin();
  delay(100);
  strip2.ClearTo(RgbwColor(0, 0, 0, 0));
  strip2.Show();
  stripStates[2].strip = (void*)&strip2;
  stripStates[2].stripType = 2;  // LedStrip2 (RMT2), synchronized with Strip 0 in software
  stripStates[2].on = false;
  stripStates[2].brightness = DEFAULT_BRIGHTNESS;
  stripStates[2].dimmingActive = false;
  stripStates[2].dimmingDirection = true;
  stripStates[2].dimmingTargetBrightness = DEFAULT_BRIGHTNESS;
  stripStates[2].isSmoothTransition = false;
  stripStates[2].lastDimmingWasIncrease = true;
  stripStates[2].blinkActive = false;
  stripStates[2].transition.active = false;
  stripStates[2].transition.randomOrder = nullptr;
  Serial.println("Strip 2 - Pin: " + String(stripConfigs[2].pin) + ", LEDs: " + String(stripConfigs[2].ledCount) + " - OK (RMT2) Kitchen extension (spice rack)");
  
  Serial.println("Initializing strip 3 on pin " + String(stripConfigs[3].pin) + " with RMT3 (Bathroom - motion activated)...");
  Serial.flush();
  strip3.Begin();
  delay(100);
  strip3.ClearTo(RgbwColor(0, 0, 0, 0));
  strip3.Show();
  stripStates[3].strip = (void*)&strip3;
  stripStates[3].stripType = 3;  // LedStrip3 (RMT3)
  stripStates[3].on = false;
  stripStates[3].brightness = DEFAULT_BRIGHTNESS;
  stripStates[3].mode = STRIP_MODE_OFF;  // Initial mode: OFF
  stripStates[3].lastAutoBrightness = 128;  // 50% initial brightness for AUTO mode
  stripStates[3].dimmingActive = false;
  stripStates[3].dimmingDirection = true;
  stripStates[3].dimmingTargetBrightness = DEFAULT_BRIGHTNESS;
  stripStates[3].isSmoothTransition = false;
  stripStates[3].lastDimmingWasIncrease = true;
  stripStates[3].blinkActive = false;
  stripStates[3].transition.active = false;
  stripStates[3].transition.randomOrder = nullptr;
  Serial.println("Strip 3 - Pin: " + String(stripConfigs[3].pin) + ", LEDs: " + String(stripConfigs[3].ledCount) + " - OK (RMT3) Bathroom (motion-activated)");
  
  Serial.println("Initializing strip 4 on pin " + String(stripConfigs[4].pin) + " with RMT4 (Bedroom - RGB type)...");
  Serial.flush();
  strip4.Begin();
  delay(100);
  strip4.ClearTo(RgbColor(0, 0, 0));  // RGB strip uses RgbColor
  strip4.Show();
  stripStates[4].strip = (void*)&strip4;
  stripStates[4].stripType = 4;  // LedStrip4 (RMT4)
  stripStates[4].on = false;
  stripStates[4].brightness = DEFAULT_BRIGHTNESS;
  stripStates[4].mode = STRIP_MODE_OFF;  // Not used for regular strips (only Strip 3 uses mode)
  stripStates[4].lastAutoBrightness = DEFAULT_BRIGHTNESS;
  stripStates[4].dimmingActive = false;
  stripStates[4].dimmingDirection = true;
  stripStates[4].dimmingTargetBrightness = DEFAULT_BRIGHTNESS;
  stripStates[4].isSmoothTransition = false;
  stripStates[4].lastDimmingWasIncrease = true;
  stripStates[4].blinkActive = false;
  stripStates[4].transition.active = false;
  stripStates[4].transition.randomOrder = nullptr;
  Serial.println("Strip 4 - Pin: " + String(stripConfigs[4].pin) + ", LEDs: " + String(stripConfigs[4].ledCount) + " - OK (RMT4, RGB type) Bedroom");
  
  // Initialize PIR sensor
  Serial.println("Initializing PIR motion sensor on pin " + String(PIR_SENSOR_PIN) + "...");
  pinMode(PIR_SENSOR_PIN, INPUT);
  Serial.println("PIR sensor - Pin: " + String(PIR_SENSOR_PIN) + " - OK");
  
  // Initialize relays
  Serial.println("Initializing relays...");
  pinMode(RELAY_PIN_0, OUTPUT);
  digitalWrite(RELAY_PIN_0, LOW);  // Start with relay OFF
  relayStates[0] = false;
  Serial.println("Relay 0 - Pin: " + String(RELAY_PIN_0) + " - OK (initialized OFF)");
  
  Serial.println("Dimming speed: " + String(DIMMING_SPEED) + " units/sec, Hold threshold: " + String(HOLD_THRESHOLD) + "ms");
  Serial.println("Transitions: " + String(TRANSITION_DURATION) + "ms");
  
  // Initialize buttons
  Serial.println("Initializing buttons...");
  Serial.flush();
  
  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
    if (buttons[i].stripIndex == 255) {
      Serial.println("Button " + String(i) + " - Pin: " + String(buttons[i].pin) + " -> Relay");
    } else {
      Serial.println("Button " + String(i) + " - Pin: " + String(buttons[i].pin) + " -> Strip " + String(buttons[i].stripIndex));
    }
    Serial.flush();
  }
  
  randomSeed(analogRead(0));
  
  // Initialize WiFi and MQTT (non-blocking, will connect in loop)
  ledControllerManager.begin();
  
  Serial.println("✅ System ready!");
  Serial.println("Click: Toggle strip ON/OFF (with random transitions)");
  Serial.println("Hold: Dim/Increase brightness\n");
}

void loop() {
  unsigned long currentTime = millis();
  
  // Update WiFi and MQTT (non-blocking)
  ledControllerManager.loop();
  
  // Process all buttons
  for (int btnIndex = 0; btnIndex < NUM_BUTTONS; btnIndex++) {
    ButtonStateMachine& btn = buttons[btnIndex];
    uint8_t stripIndex = btn.stripIndex;
    bool isRelayButton = (stripIndex == 255);  // Relay button has stripIndex 255
    
    // Read button state
    bool rawButtonReading = (digitalRead(btn.pin) == LOW);
    
    // Debounce logic (separate for each button)
    const unsigned long DEBOUNCE_DELAY = 50;
    
    if (rawButtonReading != btn.lastRawReading) {
      btn.lastDebounceTime = currentTime;
    }
    
    if (currentTime - btn.lastDebounceTime > DEBOUNCE_DELAY) {
      btn.debouncedState = rawButtonReading;
    }
    
    btn.lastRawReading = rawButtonReading;
    bool debouncedButtonState = btn.debouncedState;
    
    // Button state machine
    switch (btn.state) {
      case BUTTON_IDLE:
        if (debouncedButtonState) {
          btn.state = BUTTON_PRESSED;
          btn.pressTime = currentTime;
          Serial.println("🔘 Button " + String(btnIndex) + " pressed (IDLE -> PRESSED)");
        }
        break;
        
      case BUTTON_PRESSED:
        if (debouncedButtonState) {
          // Relay button doesn't support dimming/hold - it's just a toggle
          if (!isRelayButton && currentTime - btn.pressTime >= HOLD_THRESHOLD) {
            btn.state = BUTTON_HELD;
            if (stripStates[stripIndex].on) {
              startDimming(stripIndex);
            }
          }
        } else {
          btn.state = BUTTON_IDLE;
          if (isRelayButton) {
            // Toggle relay
            Serial.println("🔘 Button " + String(btnIndex) + " released - toggling relay");
            Serial.flush();
            toggleRelay();
          } else {
            // Toggle strip
            Serial.println("🔘 Button " + String(btnIndex) + " released - toggling strip " + String(stripIndex));
            Serial.flush();
            toggleStrip(stripIndex);
          }
        }
        break;
        
      case BUTTON_HELD:
        if (!debouncedButtonState) {
          btn.state = BUTTON_IDLE;
          if (!isRelayButton) {
            stopDimming(stripIndex);
          }
        }
        break;
    }
  }
  
  // Update all strips
  for (int i = 0; i < NUM_STRIPS; i++) {
    if (stripStates[i].transition.active) {
      updateTransition(i);
    } else {
      // Strip 3 (motion activated) has no dimming
      if (i != MOTION_STRIP_INDEX) {
        updateDimming(i);
      }
      updateBlink(i);
    }
  }
  
  // Process PIR sensor for Strip 3 (Bathroom - motion activated)
  // Only works in AUTO mode
  static unsigned long lastMotionTime = 0;
  static bool lastPirState = false;
  
  StripState& motionState = stripStates[MOTION_STRIP_INDEX];
  
  // PIR sensor only works in AUTO mode
  if (motionState.mode == STRIP_MODE_AUTO) {
    bool pirState = digitalRead(PIR_SENSOR_PIN) == HIGH;
    
    if (pirState && !lastPirState) {
      // Motion detected (rising edge)
      lastMotionTime = currentTime;
      
      if (!motionState.on) {
        // Turn on only Strip 3 (Bathroom) if not already on
        // Use lastAutoBrightness for brightness
        motionState.brightness = motionState.lastAutoBrightness;
        Serial.println("🏃 Motion detected - turning ON strip " + String(MOTION_STRIP_INDEX) + " (Bathroom, pin " + String(stripConfigs[MOTION_STRIP_INDEX].pin) + ")");
        if (DEBUG_VERBOSE) {
          Serial.println("   Kitchen strip 2 (pin 19) should remain OFF");
        }
        turnOnStrip(MOTION_STRIP_INDEX);
      } else {
        // Update last motion time
        lastMotionTime = currentTime;
      }
    }
    
    // Check if we need to turn off the strip after timeout
    if (motionState.on && !motionState.transition.active) {
      if (currentTime - lastMotionTime >= PIR_MOTION_TIMEOUT && lastMotionTime > 0) {
        Serial.println("⏱️ Motion timeout (" + String(PIR_MOTION_TIMEOUT / 1000) + "s) - turning OFF strip " + String(MOTION_STRIP_INDEX) + " (Bathroom)");
        turnOffStrip(MOTION_STRIP_INDEX);
        lastMotionTime = 0;  // Reset
      }
    }
    
    lastPirState = pirState;
  } else {
    // In OFF or ON mode, ignore PIR sensor
    lastPirState = false;
    lastMotionTime = 0;
  }
  
  delay(10);
}
