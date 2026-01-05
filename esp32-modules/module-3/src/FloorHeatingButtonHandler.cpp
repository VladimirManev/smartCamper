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
  
  // Debounce logic
  const unsigned long DEBOUNCE_DELAY = 50;
  
  if (rawButtonReading != btn.lastRawReading) {
    btn.lastDebounceTime = currentTime;
  }
  
  if (currentTime - btn.lastDebounceTime > DEBOUNCE_DELAY) {
    btn.debouncedState = rawButtonReading;
  }
  
  btn.lastRawReading = rawButtonReading;
  bool debouncedButtonState = btn.debouncedState;
  
  // Detect button press (transition from not pressed to pressed)
  if (debouncedButtonState && !btn.lastPressedState) {
    // Button just pressed - toggle circle mode (OFF <-> TEMP_CONTROL)
    if (controller) {
      controller->toggleCircleMode(circleIndex);
      if (DEBUG_SERIAL) {
        Serial.println("🔘 Button " + String(btnIndex) + " pressed - toggling circle " + String(circleIndex) + " mode");
      }
      // Publish status update after button toggle
      if (manager != nullptr) {
        manager->publishCircleStatus(circleIndex);
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

