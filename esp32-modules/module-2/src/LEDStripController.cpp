// LED Strip Controller Implementation
// Manages all LED strips (initialization, control, transitions, dimming)

#include "LEDStripController.h"
#include "ModuleManager.h"
#include "Config.h"
#include <Arduino.h>

// Static strip configuration
const StripConfig LEDStripController::stripConfigs[NUM_STRIPS] = {
  {STRIP_0_PIN, STRIP_0_LED_COUNT},   // Strip 0: Kitchen (main)
  {STRIP_1_PIN, STRIP_1_LED_COUNT},   // Strip 1: Main lighting
  {STRIP_2_PIN, STRIP_2_LED_COUNT},   // Strip 2: Kitchen (extension for spice rack, mirrors strip 0)
  {STRIP_3_PIN, STRIP_3_LED_COUNT},   // Strip 3: Bathroom (motion activated, no button, no dimming)
  {STRIP_4_PIN, STRIP_4_LED_COUNT}    // Strip 4: Bedroom (GRBW protocol)
};


// Static strip objects - using different RMT channels
static LedStrip0 strip0(STRIP_0_LED_COUNT, STRIP_0_PIN);
static LedStrip1 strip1(STRIP_1_LED_COUNT, STRIP_1_PIN);
static LedStrip2 strip2(STRIP_2_LED_COUNT, STRIP_2_PIN);
static LedStrip3 strip3(STRIP_3_LED_COUNT, STRIP_3_PIN);
static LedStrip4 strip4(STRIP_4_LED_COUNT, STRIP_4_PIN);

LEDStripController::LEDStripController(ModuleManager* moduleMgr) 
  : moduleManager(moduleMgr) {
  // Initialize strip states
  for (int i = 0; i < NUM_STRIPS; i++) {
    stripStates[i].on = false;
    stripStates[i].brightness = DEFAULT_BRIGHTNESS;
    stripStates[i].mode = STRIP_MODE_OFF;
    stripStates[i].lastAutoBrightness = DEFAULT_BRIGHTNESS;
    stripStates[i].dimmingActive = false;
    stripStates[i].dimmingDirection = true;
    stripStates[i].dimmingTargetBrightness = DEFAULT_BRIGHTNESS;
    stripStates[i].isSmoothTransition = false;
    stripStates[i].lastDimmingWasIncrease = true;
    stripStates[i].blinkActive = false;
    stripStates[i].transition.active = false;
    stripStates[i].transition.randomOrder = nullptr;
  }
  
  // Special initialization for Strip 3 (motion activated)
  stripStates[3].mode = STRIP_MODE_OFF;
  stripStates[3].lastAutoBrightness = 128;  // 50% initial brightness for AUTO mode
}

void LEDStripController::begin() {
  if (DEBUG_SERIAL) {
    Serial.println("\n\n✨ LED Strip Controller Starting...");
    Serial.println("Number of strips: " + String(NUM_STRIPS));
  }
  
  randomSeed(analogRead(0));
  
  // Initialize strip 0
  if (DEBUG_SERIAL) {
    Serial.println("Initializing strip 0 on pin " + String(stripConfigs[0].pin) + " with RMT0...");
    Serial.flush();
  }
  strip0.Begin();
  delay(100);
  strip0.ClearTo(RgbwColor(0, 0, 0, 0));
  strip0.Show();
  stripStates[0].strip = (void*)&strip0;
  stripStates[0].stripType = 0;
  if (DEBUG_SERIAL) {
    Serial.println("Strip 0 - Pin: " + String(stripConfigs[0].pin) + ", LEDs: " + String(stripConfigs[0].ledCount) + " - OK (RMT0)");
  }
  
  // Initialize strip 1
  if (DEBUG_SERIAL) {
    Serial.println("Initializing strip 1 on pin " + String(stripConfigs[1].pin) + " with RMT1...");
    Serial.flush();
  }
  strip1.Begin();
  delay(100);
  strip1.ClearTo(RgbwColor(0, 0, 0, 0));
  strip1.Show();
  stripStates[1].strip = (void*)&strip1;
  stripStates[1].stripType = 1;
  if (DEBUG_SERIAL) {
    Serial.println("Strip 1 - Pin: " + String(stripConfigs[1].pin) + ", LEDs: " + String(stripConfigs[1].ledCount) + " - OK (RMT1) Main lighting");
  }
  
  // Initialize strip 2
  if (DEBUG_SERIAL) {
    Serial.println("Initializing strip 2 on pin " + String(stripConfigs[2].pin) + " with RMT2 (Kitchen extension - spice rack, synced with Strip 0)...");
    Serial.flush();
  }
  strip2.Begin();
  delay(100);
  strip2.ClearTo(RgbwColor(0, 0, 0, 0));
  strip2.Show();
  stripStates[2].strip = (void*)&strip2;
  stripStates[2].stripType = 2;
  if (DEBUG_SERIAL) {
    Serial.println("Strip 2 - Pin: " + String(stripConfigs[2].pin) + ", LEDs: " + String(stripConfigs[2].ledCount) + " - OK (RMT2) Kitchen extension (spice rack)");
  }
  
  // Initialize strip 3
  if (DEBUG_SERIAL) {
    Serial.println("Initializing strip 3 on pin " + String(stripConfigs[3].pin) + " with RMT3 (Bathroom - motion activated)...");
    Serial.flush();
  }
  strip3.Begin();
  delay(100);
  strip3.ClearTo(RgbwColor(0, 0, 0, 0));
  strip3.Show();
  stripStates[3].strip = (void*)&strip3;
  stripStates[3].stripType = 3;
  if (DEBUG_SERIAL) {
    Serial.println("Strip 3 - Pin: " + String(stripConfigs[3].pin) + ", LEDs: " + String(stripConfigs[3].ledCount) + " - OK (RMT3) Bathroom (motion-activated)");
  }
  
  // Initialize strip 4
  if (DEBUG_SERIAL) {
    Serial.println("Initializing strip 4 on pin " + String(stripConfigs[4].pin) + " with RMT4 (Bedroom - RGBW GRBW type)...");
    Serial.flush();
  }
  strip4.Begin();
  delay(100);
  strip4.ClearTo(RgbwColor(0, 0, 0, 0));
  strip4.Show();
  stripStates[4].strip = (void*)&strip4;
  stripStates[4].stripType = 4;
  if (DEBUG_SERIAL) {
    Serial.println("Strip 4 - Pin: " + String(stripConfigs[4].pin) + ", LEDs: " + String(stripConfigs[4].ledCount) + " - OK (RMT4, RGBW GRBW type) Bedroom");
  }
  
  if (DEBUG_SERIAL) {
    Serial.println("Dimming speed: " + String(DIMMING_SPEED) + " units/sec, Hold threshold: " + String(HOLD_THRESHOLD) + "ms");
    Serial.println("Transitions: " + String(TRANSITION_DURATION) + "ms");
    Serial.println("✅ LED Strip Controller Ready!");
  }
}

// Helper functions for colors
RgbwColor LEDStripController::fixColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  return RgbwColor(g, r, b, w);  // R and G are swapped in hardware
}

RgbwColor LEDStripController::whiteColor(uint8_t brightness) {
  return RgbwColor(brightness, brightness, brightness, brightness);
}

RgbwColor LEDStripController::warmWhiteColor(uint8_t brightness) {
  // Warm white proportions: R=100%, G=90%, B=75%
  uint8_t r = brightness;
  uint8_t g = (brightness * 90) / 100;
  uint8_t b = (brightness * 75) / 100;
  return RgbwColor(r, g, b, 0);
}

RgbwColor LEDStripController::getWhiteColorForStrip(uint8_t stripIndex, uint8_t brightness) {
  if (stripIndex == 4) {
    return warmWhiteColor(brightness);  // Strip 4 (Bedroom) uses warm white
  } else {
    return whiteColor(brightness);
  }
}

// Helper functions for working with different strip types
void LEDStripController::setPixelColor(uint8_t stripIndex, int pixelIndex, RgbwColor color) {
  if (stripIndex >= NUM_STRIPS) return;
  StripState& state = stripStates[stripIndex];
  switch (state.stripType) {
    case 0: ((LedStrip0*)state.strip)->SetPixelColor(pixelIndex, color); break;
    case 1: ((LedStrip1*)state.strip)->SetPixelColor(pixelIndex, color); break;
    case 2: ((LedStrip2*)state.strip)->SetPixelColor(pixelIndex, color); break;
    case 3: ((LedStrip3*)state.strip)->SetPixelColor(pixelIndex, color); break;
    case 4: ((LedStrip4*)state.strip)->SetPixelColor(pixelIndex, color); break;
  }
}

void LEDStripController::clearStrip(uint8_t stripIndex, RgbwColor color) {
  if (stripIndex >= NUM_STRIPS) return;
  StripState& state = stripStates[stripIndex];
  switch (state.stripType) {
    case 0: ((LedStrip0*)state.strip)->ClearTo(color); break;
    case 1: ((LedStrip1*)state.strip)->ClearTo(color); break;
    case 2: ((LedStrip2*)state.strip)->ClearTo(color); break;
    case 3: ((LedStrip3*)state.strip)->ClearTo(color); break;
    case 4: ((LedStrip4*)state.strip)->ClearTo(color); break;
  }
}

void LEDStripController::showStrip(uint8_t stripIndex) {
  if (stripIndex >= NUM_STRIPS) return;
  StripState& state = stripStates[stripIndex];
  switch (state.stripType) {
    case 0: ((LedStrip0*)state.strip)->Show(); break;
    case 1: ((LedStrip1*)state.strip)->Show(); break;
    case 2: ((LedStrip2*)state.strip)->Show(); break;
    case 3: ((LedStrip3*)state.strip)->Show(); break;
    case 4: ((LedStrip4*)state.strip)->Show(); break;
  }
}

// Kitchen synchronization (Strip 0 and Strip 2)
void LEDStripController::syncKitchenExtension(uint8_t mainStripIndex) {
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
void LEDStripController::updateStrip(uint8_t stripIndex) {
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

void LEDStripController::transitionOnCenterToEdges(uint8_t stripIndex) {
  StripState& state = stripStates[stripIndex];
  TransitionState& trans = state.transition;
  uint16_t ledCount = stripConfigs[stripIndex].ledCount;
  
  unsigned long elapsed = millis() - trans.startTime;
  float progress = (float)elapsed / TRANSITION_DURATION;
  if (progress > 1.0) progress = 1.0;
  
  int center = ledCount / 2;
  int maxDistance = center;
  int currentDistance = (int)(maxDistance * progress);
  
  clearStrip(stripIndex, RgbwColor(0, 0, 0, 0));
  for (int i = 0; i <= currentDistance; i++) {
    if (center - i >= 0) {
      setPixelColor(stripIndex, center - i, getWhiteColorForStrip(stripIndex, trans.targetBrightness));
    }
    if (center + i < ledCount) {
      setPixelColor(stripIndex, center + i, getWhiteColorForStrip(stripIndex, trans.targetBrightness));
    }
  }
  showStrip(stripIndex);
  
  if (progress >= 1.0) {
    trans.active = false;
  }
}

void LEDStripController::transitionOnRandomLeds(uint8_t stripIndex) {
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
  clearStrip(stripIndex, RgbwColor(0, 0, 0, 0));
  
  for (int i = 0; i < targetCount && i < ledCount; i++) {
    setPixelColor(stripIndex, trans.randomOrder[i], 
                  getWhiteColorForStrip(stripIndex, trans.targetBrightness));
  }
  showStrip(stripIndex);
  
  if (progress >= 1.0) {
    delete[] trans.randomOrder;
    trans.randomOrder = nullptr;
    trans.active = false;
  }
}

void LEDStripController::transitionOnLeftToRight(uint8_t stripIndex) {
  StripState& state = stripStates[stripIndex];
  TransitionState& trans = state.transition;
  uint16_t ledCount = stripConfigs[stripIndex].ledCount;
  
  unsigned long elapsed = millis() - trans.startTime;
  float progress = (float)elapsed / TRANSITION_DURATION;
  if (progress > 1.0) progress = 1.0;
  
  int currentEnd = (int)(ledCount * progress);
  
  clearStrip(stripIndex, RgbwColor(0, 0, 0, 0));
  for (int i = 0; i < currentEnd; i++) {
    setPixelColor(stripIndex, i, getWhiteColorForStrip(stripIndex, trans.targetBrightness));
  }
  showStrip(stripIndex);
  
  if (progress >= 1.0) {
    trans.active = false;
  }
}

void LEDStripController::transitionOnEdgesToCenter(uint8_t stripIndex) {
  StripState& state = stripStates[stripIndex];
  TransitionState& trans = state.transition;
  uint16_t ledCount = stripConfigs[stripIndex].ledCount;
  
  unsigned long elapsed = millis() - trans.startTime;
  float progress = (float)elapsed / TRANSITION_DURATION;
  if (progress > 1.0) progress = 1.0;
  
  int center = ledCount / 2;
  int maxDistance = center;
  int currentDistance = (int)(maxDistance * (1.0 - progress));
  
  clearStrip(stripIndex, RgbwColor(0, 0, 0, 0));
  for (int i = 0; i <= maxDistance - currentDistance; i++) {
    if (center - i >= 0) {
      setPixelColor(stripIndex, center - i, getWhiteColorForStrip(stripIndex, trans.targetBrightness));
    }
    if (center + i < ledCount) {
      setPixelColor(stripIndex, center + i, getWhiteColorForStrip(stripIndex, trans.targetBrightness));
    }
  }
  showStrip(stripIndex);
  
  if (progress >= 1.0) {
    trans.active = false;
  }
}

// ============================================================================
// TURN OFF TRANSITIONS
// ============================================================================

void LEDStripController::transitionOffEdgesToCenter(uint8_t stripIndex) {
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
    setPixelColor(stripIndex, i, getWhiteColorForStrip(stripIndex, trans.targetBrightness));
  }
  
  for (int i = 0; i < currentDistance; i++) {
    if (i < ledCount) {
      setPixelColor(stripIndex, i, RgbwColor(0, 0, 0, 0));
    }
    if (ledCount - 1 - i >= 0) {
      setPixelColor(stripIndex, ledCount - 1 - i, RgbwColor(0, 0, 0, 0));
    }
  }
  
  // For odd number of LEDs, clear the middle LED when we reach the center
  if (ledCount % 2 == 1 && currentDistance >= center) {
    setPixelColor(stripIndex, center, RgbwColor(0, 0, 0, 0));
  }
  
  showStrip(stripIndex);
  
  if (progress >= 1.0) {
    trans.active = false;
  }
}

void LEDStripController::transitionOffRandomLeds(uint8_t stripIndex) {
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
    setPixelColor(stripIndex, i, getWhiteColorForStrip(stripIndex, trans.targetBrightness));
  }
  
  for (int i = 0; i < offCount && i < ledCount; i++) {
    setPixelColor(stripIndex, trans.randomOrder[i], RgbwColor(0, 0, 0, 0));
  }
  showStrip(stripIndex);
  
  if (progress >= 1.0) {
    delete[] trans.randomOrder;
    trans.randomOrder = nullptr;
    trans.active = false;
  }
}

void LEDStripController::transitionOffLeftToRight(uint8_t stripIndex) {
  StripState& state = stripStates[stripIndex];
  TransitionState& trans = state.transition;
  uint16_t ledCount = stripConfigs[stripIndex].ledCount;
  
  unsigned long elapsed = millis() - trans.startTime;
  float progress = (float)elapsed / TRANSITION_DURATION;
  if (progress > 1.0) progress = 1.0;
  
  int currentEnd = (int)(ledCount * progress);
  
  for (int i = 0; i < ledCount; i++) {
    setPixelColor(stripIndex, i, getWhiteColorForStrip(stripIndex, trans.targetBrightness));
  }
  
  for (int i = 0; i < currentEnd; i++) {
    setPixelColor(stripIndex, i, RgbwColor(0, 0, 0, 0));
  }
  showStrip(stripIndex);
  
  if (progress >= 1.0) {
    trans.active = false;
  }
}

void LEDStripController::transitionOffCenterToEdges(uint8_t stripIndex) {
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
    setPixelColor(stripIndex, i, getWhiteColorForStrip(stripIndex, trans.targetBrightness));
  }
  
  for (int i = 0; i <= currentDistance; i++) {
    if (center - i >= 0) {
      setPixelColor(stripIndex, center - i, RgbwColor(0, 0, 0, 0));
    }
    if (center + i < ledCount) {
      setPixelColor(stripIndex, center + i, RgbwColor(0, 0, 0, 0));
    }
  }
  showStrip(stripIndex);
  
  if (progress >= 1.0) {
    trans.active = false;
  }
}

// ============================================================================
// TRANSITION MANAGEMENT
// ============================================================================

void LEDStripController::startTransition(uint8_t stripIndex, bool turningOn) {
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

void LEDStripController::updateTransition(uint8_t stripIndex) {
  if (stripIndex >= NUM_STRIPS) return;
  
  StripState& state = stripStates[stripIndex];
  TransitionState& trans = state.transition;
  
  if (!trans.active) return;
  
  // Execute transition function using switch
  if (trans.type < NUM_ON_TRANSITIONS) {
    switch (trans.type) {
      case TRANSITION_ON_CENTER_TO_EDGES:
        transitionOnCenterToEdges(stripIndex);
        break;
      case TRANSITION_ON_RANDOM_LEDS:
        transitionOnRandomLeds(stripIndex);
        break;
      case TRANSITION_ON_LEFT_TO_RIGHT:
        transitionOnLeftToRight(stripIndex);
        break;
      case TRANSITION_ON_EDGES_TO_CENTER:
        transitionOnEdgesToCenter(stripIndex);
        break;
      default:
        break;
    }
  } else {
    int offIndex = trans.type - NUM_ON_TRANSITIONS;
    switch (offIndex) {
      case 0:  // TRANSITION_OFF_EDGES_TO_CENTER
        transitionOffEdgesToCenter(stripIndex);
        break;
      case 1:  // TRANSITION_OFF_RANDOM_LEDS
        transitionOffRandomLeds(stripIndex);
        break;
      case 2:  // TRANSITION_OFF_LEFT_TO_RIGHT
        transitionOffLeftToRight(stripIndex);
        break;
      case 3:  // TRANSITION_OFF_CENTER_TO_EDGES
        transitionOffCenterToEdges(stripIndex);
        break;
      default:
        break;
    }
  }
  
  // Kitchen: synchronize extension strip - execute the same transition
  if (stripIndex == 0 && stripStates[2].transition.active) {
    TransitionState& extTrans = stripStates[2].transition;
    if (extTrans.type < NUM_ON_TRANSITIONS) {
      switch (extTrans.type) {
        case TRANSITION_ON_CENTER_TO_EDGES:
          transitionOnCenterToEdges(2);
          break;
        case TRANSITION_ON_RANDOM_LEDS:
          transitionOnRandomLeds(2);
          break;
        case TRANSITION_ON_LEFT_TO_RIGHT:
          transitionOnLeftToRight(2);
          break;
        case TRANSITION_ON_EDGES_TO_CENTER:
          transitionOnEdgesToCenter(2);
          break;
        default:
          break;
      }
    } else {
      int offIndex = extTrans.type - NUM_ON_TRANSITIONS;
      switch (offIndex) {
        case 0:
          transitionOffEdgesToCenter(2);
          break;
        case 1:
          transitionOffRandomLeds(2);
          break;
        case 2:
          transitionOffLeftToRight(2);
          break;
        case 3:
          transitionOffCenterToEdges(2);
          break;
        default:
          break;
      }
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

void LEDStripController::updateBlink(uint8_t stripIndex) {
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
      setPixelColor(stripIndex, i, getWhiteColorForStrip(stripIndex, currentBrightness));
    }
    showStrip(stripIndex);
    
    // Kitchen: synchronize extension strip - use the same brightnessFactor
    if (stripIndex == 0 && stripStates[2].blinkActive && stripStates[2].on) {
      StripState& extState = stripStates[2];
      uint8_t extBrightness = (uint8_t)(extState.savedBrightnessForBlink * brightnessFactor);
      for (int i = 0; i < stripConfigs[2].ledCount; i++) {
        setPixelColor(2, i, whiteColor(extBrightness));
      }
      showStrip(2);
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

void LEDStripController::updateDimming(uint8_t stripIndex) {
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
    // Smooth transition completed - we'll publish status externally
    state.dimmingActive = false;
    state.isSmoothTransition = false;
    state.brightness = targetBrightness;
    updateStrip(stripIndex);
    syncKitchenExtension(stripIndex);
  } else {
    // Still transitioning
    state.brightness = newBrightness;
    updateStrip(stripIndex);
    // Kitchen: synchronize extension strip during dimming
    syncKitchenExtension(stripIndex);
  }
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void LEDStripController::loop() {
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
}

// ============================================================================
// MAIN CONTROL FUNCTIONS (called from button, MQTT, sensors)
// ============================================================================

void LEDStripController::turnOnStrip(uint8_t stripIndex) {
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
}

void LEDStripController::turnOffStrip(uint8_t stripIndex) {
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
}

void LEDStripController::toggleStrip(uint8_t stripIndex) {
  if (stripIndex >= NUM_STRIPS) {
    Serial.println("ERROR: toggleStrip called with invalid stripIndex: " + String(stripIndex));
    return;
  }
  
  StripState& state = stripStates[stripIndex];
  if (DEBUG_VERBOSE) {
    Serial.println("🔄 toggleStrip(" + String(stripIndex) + ") - current state: " + String(state.on ? "ON" : "OFF"));
  }
  
  if (state.on) {
    turnOffStrip(stripIndex);
  } else {
    turnOnStrip(stripIndex);
  }
}

void LEDStripController::setStripMode(uint8_t stripIndex, StripMode mode) {
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
}

void LEDStripController::startDimming(uint8_t stripIndex) {
  if (stripIndex >= NUM_STRIPS) return;
  
  // Strip 3 (motion activated) has no dimming
  if (stripIndex == MOTION_STRIP_INDEX) return;
  
  StripState& state = stripStates[stripIndex];
  if (!state.on || state.dimmingActive) return;
  
  // Stop any active transition when dimming starts (button has priority)
  if (state.transition.active) {
    state.transition.active = false;
    // Update strip to current state before starting dimming
    updateStrip(stripIndex);
  }
  
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

void LEDStripController::stopDimming(uint8_t stripIndex) {
  if (stripIndex >= NUM_STRIPS) return;
  
  StripState& state = stripStates[stripIndex];
  state.dimmingActive = false;
  Serial.println("🔆 Strip " + String(stripIndex) + " dimming stopped (Brightness: " + String(state.brightness) + ")");
  
  // Kitchen: synchronize extension strip
  syncKitchenExtension(stripIndex);
}

// Set brightness smoothly (for MQTT commands)
void LEDStripController::setBrightnessSmooth(uint8_t stripIndex, uint8_t targetBrightness) {
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

// ============================================================================
// GETTERS
// ============================================================================

StripState& LEDStripController::getStripState(uint8_t stripIndex) {
  if (stripIndex >= NUM_STRIPS) stripIndex = 0;  // Return first strip if invalid
  return stripStates[stripIndex];
}

const StripState& LEDStripController::getStripState(uint8_t stripIndex) const {
  if (stripIndex >= NUM_STRIPS) stripIndex = 0;  // Return first strip if invalid
  return stripStates[stripIndex];
}

bool LEDStripController::isStripOn(uint8_t stripIndex) const {
  if (stripIndex >= NUM_STRIPS) return false;
  return stripStates[stripIndex].on;
}

uint8_t LEDStripController::getBrightness(uint8_t stripIndex) const {
  if (stripIndex >= NUM_STRIPS) return 0;
  return stripStates[stripIndex].brightness;
}

void LEDStripController::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("📊 LED Strip Controller Status:");
    for (int i = 0; i < NUM_STRIPS; i++) {
      const StripState& state = stripStates[i];
      Serial.println("  Strip " + String(i) + ": " + String(state.on ? "ON" : "OFF") + 
                     ", Brightness: " + String(state.brightness) + 
                     ", Dimming: " + String(state.dimmingActive ? "Active" : "Inactive") +
                     ", Transition: " + String(state.transition.active ? "Active" : "Inactive"));
    }
  }
}

