// LED Strip Controller
// Manages all LED strips (initialization, control, transitions, dimming)

#ifndef LED_STRIP_CONTROLLER_H
#define LED_STRIP_CONTROLLER_H

#include "Config.h"
#include "StripState.h"
#include <NeoPixelBus.h>
#include <Arduino.h>

// Forward declaration
class ModuleManager;

// NeoPixelBus strip type definitions - using different RMT channels
typedef NeoPixelBus<NeoRgbwFeature, NeoEsp32Rmt0Ws2812xMethod> LedStrip0;
typedef NeoPixelBus<NeoRgbwFeature, NeoEsp32Rmt1Ws2812xMethod> LedStrip1;
typedef NeoPixelBus<NeoRgbwFeature, NeoEsp32Rmt2Ws2812xMethod> LedStrip2;
typedef NeoPixelBus<NeoRgbwFeature, NeoEsp32Rmt3Ws2812xMethod> LedStrip3;
typedef NeoPixelBus<NeoGrbwFeature, NeoEsp32Rmt4Ws2812xMethod> LedStrip4;

// Strip configuration structure
struct StripConfig {
  uint8_t pin;
  uint16_t ledCount;
};

class LEDStripController {
private:
  ModuleManager* moduleManager;  // Reference to module manager (not owned)
  
  // Strip configurations
  static const StripConfig stripConfigs[NUM_STRIPS];
  
  // NeoPixelBus strip objects - using different RMT channels (stored as void* in StripState)
  // These are created in begin() and stored in stripStates
  
  StripState stripStates[NUM_STRIPS];
  
  // Helper functions for different strip types
  void setPixelColor(uint8_t stripIndex, int pixelIndex, RgbwColor color);
  void clearStrip(uint8_t stripIndex, RgbwColor color);
  void showStrip(uint8_t stripIndex);
  
  // Helper functions for colors
  RgbwColor fixColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w);
  RgbwColor whiteColor(uint8_t brightness);
  RgbwColor warmWhiteColor(uint8_t brightness);
  RgbwColor getWhiteColorForStrip(uint8_t stripIndex, uint8_t brightness);
  
  // Transition functions
  void transitionOnCenterToEdges(uint8_t stripIndex);
  void transitionOnRandomLeds(uint8_t stripIndex);
  void transitionOnLeftToRight(uint8_t stripIndex);
  void transitionOnEdgesToCenter(uint8_t stripIndex);
  void transitionOffEdgesToCenter(uint8_t stripIndex);
  void transitionOffRandomLeds(uint8_t stripIndex);
  void transitionOffLeftToRight(uint8_t stripIndex);
  void transitionOffCenterToEdges(uint8_t stripIndex);
  
  void startTransition(uint8_t stripIndex, bool turningOn);
  void updateTransition(uint8_t stripIndex);
  
  // Dimming functions
  void updateDimming(uint8_t stripIndex);
  
  // Blinking functions
  void updateBlink(uint8_t stripIndex);
  
  // Kitchen synchronization (Strip 0 and Strip 2)
  void syncKitchenExtension(uint8_t mainStripIndex);
  
  // Update strip with current brightness
  void updateStrip(uint8_t stripIndex);

public:
  LEDStripController(ModuleManager* moduleMgr);
  
  // Initialization
  void begin();
  
  // Main loop - call this in your main loop()
  void loop();
  
  // Control functions (public API)
  void turnOnStrip(uint8_t stripIndex);
  void turnOffStrip(uint8_t stripIndex);
  void toggleStrip(uint8_t stripIndex);
  void setBrightnessSmooth(uint8_t stripIndex, uint8_t targetBrightness);
  void setStripMode(uint8_t stripIndex, StripMode mode);
  void startDimming(uint8_t stripIndex);
  void stopDimming(uint8_t stripIndex);
  
  // Getters
  StripState& getStripState(uint8_t stripIndex);
  const StripState& getStripState(uint8_t stripIndex) const;
  bool isStripOn(uint8_t stripIndex) const;
  uint8_t getBrightness(uint8_t stripIndex) const;
  
  // Strip configuration getter
  static const StripConfig* getStripConfigs() { return stripConfigs; }
  
  // Status
  void printStatus() const;
};

#endif

