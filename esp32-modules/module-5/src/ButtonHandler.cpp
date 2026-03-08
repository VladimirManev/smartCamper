// Button Handler Implementation

#include "ButtonHandler.h"
#include "RelayController.h"
#include "ApplianceManager.h"
#include "Config.h"

ButtonHandler::ButtonHandler(RelayController* relayCtrl) 
  : relayController(relayCtrl), applianceManager(nullptr) {
  // Initialize button state machines
  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].state = BUTTON_IDLE;
    buttons[i].pressTime = 0;
    if (i == 0) {
      buttons[i].pin = BUTTON_PIN_0;
    } else if (i == 1) {
      buttons[i].pin = BUTTON_PIN_1;
    } else if (i == 2) {
      buttons[i].pin = BUTTON_PIN_2;
    } else if (i == 3) {
      buttons[i].pin = BUTTON_PIN_3;
    } else if (i == 4) {
      buttons[i].pin = BUTTON_PIN_4;
    } else {
      buttons[i].pin = BUTTON_PIN_5;
    }
    buttons[i].relayIndex = buttonRelayMap[i];
    buttons[i].lastRawReading = false;
    buttons[i].lastDebounceTime = 0;
    buttons[i].debouncedState = false;
  }
}

void ButtonHandler::setApplianceManager(ApplianceManager* appMgr) {
  applianceManager = appMgr;
}

void ButtonHandler::begin() {
  if (DEBUG_SERIAL) {
    Serial.println("🔘 Button Handler Starting...");
  }
  
  // Initialize button pins
  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
    Serial.println("Button " + String(i) + " - Pin: " + String(buttons[i].pin) + " -> Relay " + String(buttons[i].relayIndex));
  }
  
  if (DEBUG_SERIAL) {
    Serial.println("✅ Button Handler Ready!");
  }
}

void ButtonHandler::processButton(uint8_t btnIndex, unsigned long currentTime) {
  if (btnIndex >= NUM_BUTTONS) return;
  
  ButtonStateMachine& btn = buttons[btnIndex];
  uint8_t relayIndex = btn.relayIndex;
  
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
      if (!debouncedButtonState) {
        btn.state = BUTTON_IDLE;
        // Toggle relay
        Serial.println("🔘 Button " + String(btnIndex) + " released - toggling relay " + String(relayIndex));
        Serial.flush();
        if (relayController) {
          relayController->toggleRelay(relayIndex);
          // Publish relay status update
          if (applianceManager) {
            applianceManager->publishRelayStatus();
          }
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
    // Check if button is in PRESSED state
    if (btn.state == BUTTON_PRESSED) {
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
      }
      Serial.println("  Button " + String(i) + " (Pin " + String(btn.pin) + " -> Relay " + String(btn.relayIndex) + "): " + stateStr);
    }
  }
}
