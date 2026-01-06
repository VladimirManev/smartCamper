// Floor Heating Button Handler Implementation

#include "FloorHeatingButtonHandler.h"
#include "FloorHeatingController.h"
#include "FloorHeatingManager.h"
#include "Config.h"

FloorHeatingButtonHandler::FloorHeatingButtonHandler(FloorHeatingController* ctrl) 
  : controller(ctrl), manager(nullptr) {
  // Initialize button state machines
  uint8_t buttonPins[NUM_HEATING_CIRCLES] = {
    HEATING_BUTTON_PIN_0,
    HEATING_BUTTON_PIN_1,
    HEATING_BUTTON_PIN_2,
    HEATING_BUTTON_PIN_3
  };
  
  for (int i = 0; i < NUM_HEATING_CIRCLES; i++) {
    buttons[i].pin = buttonPins[i];
    buttons[i].circleIndex = i;
    buttons[i].lastRawReading = false;
    buttons[i].lastDebounceTime = 0;
    buttons[i].debouncedState = false;
    buttons[i].lastPressedState = false;
  }
}

void FloorHeatingButtonHandler::setController(FloorHeatingController* ctrl) {
  controller = ctrl;
}

void FloorHeatingButtonHandler::setManager(FloorHeatingManager* mgr) {
  manager = mgr;
}

void FloorHeatingButtonHandler::begin() {
  if (DEBUG_SERIAL) {
    Serial.println("🔘 Floor Heating Button Handler Starting...");
  }
  
  // Initialize button pins
  for (int i = 0; i < NUM_HEATING_CIRCLES; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
    if (DEBUG_SERIAL) {
      Serial.println("Button " + String(i) + " - Pin: " + String(buttons[i].pin) + " -> Circle " + String(buttons[i].circleIndex));
    }
  }
  
  if (DEBUG_SERIAL) {
    Serial.println("✅ Floor Heating Button Handler Ready!");
  }
}

void FloorHeatingButtonHandler::loop() {
  unsigned long currentTime = millis();
  
  // Process all buttons
  for (int btnIndex = 0; btnIndex < NUM_HEATING_CIRCLES; btnIndex++) {
    processButton(btnIndex, currentTime);
  }
}

void FloorHeatingButtonHandler::processButton(uint8_t btnIndex, unsigned long currentTime) {
  if (btnIndex >= NUM_HEATING_CIRCLES) return;
  
  ButtonState& btn = buttons[btnIndex];
  uint8_t circleIndex = btn.circleIndex;
  
  // Read button state (LOW = pressed, HIGH = released, because of INPUT_PULLUP)
  bool rawButtonReading = (digitalRead(btn.pin) == LOW);
  
  // Debounce logic - increased delay for better reliability
  const unsigned long DEBOUNCE_DELAY = 100;  // Increased from 50ms for better reliability
  const unsigned long MIN_PRESS_INTERVAL = 300;  // Minimum time between button presses (ms)
  
  // If button state changed, reset debounce timer
  if (rawButtonReading != btn.lastRawReading) {
    btn.lastDebounceTime = currentTime;
  }
  
  // Update debounced state only after debounce delay
  if (currentTime - btn.lastDebounceTime > DEBOUNCE_DELAY) {
    // Only update if state actually changed
    if (rawButtonReading != btn.debouncedState) {
      btn.debouncedState = rawButtonReading;
    }
  }
  
  btn.lastRawReading = rawButtonReading;
  bool debouncedButtonState = btn.debouncedState;
  
  // Detect button press (transition from not pressed to pressed)
  // Also check minimum interval to prevent multiple triggers
  static unsigned long lastButtonPressTime[NUM_HEATING_CIRCLES] = {0};
  
  if (debouncedButtonState && !btn.lastPressedState) {
    // Check if enough time has passed since last press
    if (currentTime - lastButtonPressTime[btnIndex] >= MIN_PRESS_INTERVAL) {
      // Button just pressed - toggle circle mode (OFF <-> TEMP_CONTROL)
      if (controller) {
        // Toggle mode first (fast operation)
        controller->toggleCircleMode(circleIndex);
        lastButtonPressTime[btnIndex] = currentTime;  // Record press time
        
        if (DEBUG_SERIAL) {
          Serial.println("🔘 Button " + String(btnIndex) + " pressed - toggling circle " + String(circleIndex) + " mode");
        }
        
        // Note: publishCircleStatus is already called from setCircleMode, so we don't need to call it here
      }
    } else {
      if (DEBUG_SERIAL) {
        Serial.println("⚠️ Button " + String(btnIndex) + " press ignored - too soon after last press");
      }
    }
  }
  
  // Update last pressed state
  btn.lastPressedState = debouncedButtonState;
}

void FloorHeatingButtonHandler::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("📊 Floor Heating Button Handler Status:");
    for (int i = 0; i < NUM_HEATING_CIRCLES; i++) {
      const ButtonState& btn = buttons[i];
      String stateStr = btn.debouncedState ? "PRESSED" : "RELEASED";
      Serial.println("  Button " + String(i) + " (Pin " + String(btn.pin) + "): " + stateStr + " -> Circle " + String(btn.circleIndex));
    }
  }
}

