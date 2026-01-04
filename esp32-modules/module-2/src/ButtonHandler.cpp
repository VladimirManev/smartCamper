// Button Handler Implementation

#include "ButtonHandler.h"
#include "LEDStripController.h"
#include "RelayController.h"
#include "Config.h"

ButtonHandler::ButtonHandler(LEDStripController* ledCtrl, RelayController* relayCtrl) 
  : ledController(ledCtrl), relayController(relayCtrl) {
  // Initialize button state machines
  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].state = BUTTON_IDLE;
    buttons[i].pressTime = 0;
    buttons[i].pin = buttonConfigs[i].pin;
    buttons[i].stripIndex = buttonConfigs[i].stripIndex;
    buttons[i].lastRawReading = false;
    buttons[i].lastDebounceTime = 0;
    buttons[i].debouncedState = false;
  }
}

void ButtonHandler::begin() {
  if (DEBUG_SERIAL) {
    Serial.println("🔘 Button Handler Starting...");
  }
  
  // Initialize button pins
  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
    if (buttons[i].stripIndex == 255) {
      Serial.println("Button " + String(i) + " - Pin: " + String(buttons[i].pin) + " -> Relay");
    } else {
      Serial.println("Button " + String(i) + " - Pin: " + String(buttons[i].pin) + " -> Strip " + String(buttons[i].stripIndex));
    }
  }
  
  if (DEBUG_SERIAL) {
    Serial.println("✅ Button Handler Ready!");
  }
}

void ButtonHandler::processButton(uint8_t btnIndex, unsigned long currentTime) {
  if (btnIndex >= NUM_BUTTONS) return;
  
  ButtonStateMachine& btn = buttons[btnIndex];
  uint8_t stripIndex = btn.stripIndex;
  bool isRelayButton = (stripIndex == 255);  // Relay button has stripIndex 255
  
  // Read button state
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
          if (ledController && ledController->isStripOn(stripIndex)) {
            ledController->startDimming(stripIndex);
          }
        }
      } else {
        btn.state = BUTTON_IDLE;
        if (isRelayButton) {
          // Toggle relay
          Serial.println("🔘 Button " + String(btnIndex) + " released - toggling relay");
          Serial.flush();
          if (relayController) {
            relayController->toggleRelay(0);
          }
        } else {
          // Toggle strip
          Serial.println("🔘 Button " + String(btnIndex) + " released - toggling strip " + String(stripIndex));
          Serial.flush();
          if (ledController) {
            ledController->toggleStrip(stripIndex);
          }
        }
      }
      break;
      
    case BUTTON_HELD:
      if (!debouncedButtonState) {
        btn.state = BUTTON_IDLE;
        if (!isRelayButton && ledController) {
          ledController->stopDimming(stripIndex);
        }
      }
      break;
  }
}

void ButtonHandler::loop() {
  unsigned long currentTime = millis();
  
  // Process all buttons
  for (int btnIndex = 0; btnIndex < NUM_BUTTONS; btnIndex++) {
    processButton(btnIndex, currentTime);
  }
}

bool ButtonHandler::isAnyButtonPressed() const {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    const ButtonStateMachine& btn = buttons[i];
    // Check if button is in PRESSED or HELD state
    if (btn.state == BUTTON_PRESSED || btn.state == BUTTON_HELD) {
      return true;
    }
  }
  return false;
}

void ButtonHandler::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("📊 Button Handler Status:");
    for (int i = 0; i < NUM_BUTTONS; i++) {
      const ButtonStateMachine& btn = buttons[i];
      String stateStr;
      switch (btn.state) {
        case BUTTON_IDLE: stateStr = "IDLE"; break;
        case BUTTON_PRESSED: stateStr = "PRESSED"; break;
        case BUTTON_HELD: stateStr = "HELD"; break;
      }
      Serial.println("  Button " + String(i) + " (Pin " + String(btn.pin) + "): " + stateStr);
    }
  }
}

