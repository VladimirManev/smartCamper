// Table Controller Implementation
// Controls table lift with two relays and buttons

#include "TableController.h"
#include "MQTTManager.h"
#include <ArduinoJson.h>
#include <Arduino.h>

TableController::TableController(int relayUp, int relayDown, int btnUp, int btnDown, MQTTManager* mqtt)
  : relayUpPin(relayUp), relayDownPin(relayDown), buttonUpPin(btnUp), buttonDownPin(btnDown),
    mqttManager(mqtt), relayUpActive(false), relayDownActive(false),
    lastButtonUpState(false), lastButtonDownState(false),
    debouncedButtonUpState(false), debouncedButtonDownState(false),
    lastDebounceUpTime(0), lastDebounceDownTime(0),
    lastClickUpTime(0), lastClickDownTime(0),
    waitingForDoubleClickUp(false), waitingForDoubleClickDown(false),
    pendingMoveUpTime(0), pendingMoveDownTime(0),
    pendingMoveUp(false), pendingMoveDown(false),
    autoMoving(false), autoMoveStartTime(0), autoMoveDirection(false) {
}

void TableController::begin() {
  // Initialize relay pins as outputs
  pinMode(relayUpPin, OUTPUT);
  pinMode(relayDownPin, OUTPUT);
  
  // Turn off relays initially (LOW = relay off)
  digitalWrite(relayUpPin, LOW);
  digitalWrite(relayDownPin, LOW);
  relayUpActive = false;
  relayDownActive = false;
  
  // Initialize button pins as inputs with pull-up
  pinMode(buttonUpPin, INPUT_PULLUP);
  pinMode(buttonDownPin, INPUT_PULLUP);
  
  lastButtonUpState = (digitalRead(buttonUpPin) == LOW);
  lastButtonDownState = (digitalRead(buttonDownPin) == LOW);
  debouncedButtonUpState = lastButtonUpState;
  debouncedButtonDownState = lastButtonDownState;
  lastDebounceUpTime = millis();
  lastDebounceDownTime = millis();
  
  if (DEBUG_SERIAL) {
    Serial.println("🔧 TableController initialized");
    Serial.println("  Relay Up pin: " + String(relayUpPin));
    Serial.println("  Relay Down pin: " + String(relayDownPin));
    Serial.println("  Button Up pin: " + String(buttonUpPin));
    Serial.println("  Button Down pin: " + String(buttonDownPin));
  }
}

void TableController::moveUp() {
  // If auto-moving, stop immediately on any button press
  if (autoMoving) {
    stop();
    if (DEBUG_SERIAL) {
      Serial.println("⏹️ TableController: Auto movement stopped by move up command");
    }
    return;
  }
  
  // Safety: if down relay is active, ignore up command
  if (relayDownActive) {
    if (DEBUG_SERIAL) {
      Serial.println("⚠️ TableController: Cannot move up - down relay is active");
    }
    return;
  }
  
  relayUpActive = true;
  updateRelays();
  publishStatus();
  
  if (DEBUG_SERIAL) {
    Serial.println("⬆️ TableController: Moving up");
  }
}

void TableController::moveDown() {
  // If auto-moving, stop immediately on any button press
  if (autoMoving) {
    stop();
    if (DEBUG_SERIAL) {
      Serial.println("⏹️ TableController: Auto movement stopped by move down command");
    }
    return;
  }
  
  // Safety: if up relay is active, ignore down command
  if (relayUpActive) {
    if (DEBUG_SERIAL) {
      Serial.println("⚠️ TableController: Cannot move down - up relay is active");
    }
    return;
  }
  
  relayDownActive = true;
  updateRelays();
  publishStatus();
  
  if (DEBUG_SERIAL) {
    Serial.println("⬇️ TableController: Moving down");
  }
}

void TableController::stop() {
  relayUpActive = false;
  relayDownActive = false;
  autoMoving = false;
  updateRelays();
  publishStatus();
  
  if (DEBUG_SERIAL) {
    Serial.println("⏹️ TableController: Stopped");
  }
}

void TableController::stopMovement() {
  stop();
}

void TableController::moveUpAuto(int durationMs) {
  // Safety: if down relay is active, ignore up command
  if (relayDownActive) {
    if (DEBUG_SERIAL) {
      Serial.println("⚠️ TableController: Cannot move up - down relay is active");
    }
    return;
  }
  
  relayUpActive = true;
  autoMoving = true;
  autoMoveDirection = true;  // up
  autoMoveStartTime = millis();
  updateRelays();
  publishStatus();
  
  if (DEBUG_SERIAL) {
    Serial.println("⬆️ TableController: Auto moving up for " + String(durationMs) + "ms");
  }
}

void TableController::moveDownAuto(int durationMs) {
  // Safety: if up relay is active, ignore down command
  if (relayUpActive) {
    if (DEBUG_SERIAL) {
      Serial.println("⚠️ TableController: Cannot move down - up relay is active");
    }
    return;
  }
  
  relayDownActive = true;
  autoMoving = true;
  autoMoveDirection = false;  // down
  autoMoveStartTime = millis();
  updateRelays();
  publishStatus();
  
  if (DEBUG_SERIAL) {
    Serial.println("⬇️ TableController: Auto moving down for " + String(durationMs) + "ms");
  }
}

void TableController::updateRelays() {
  // Update relay states (HIGH = relay on, LOW = relay off)
  digitalWrite(relayUpPin, relayUpActive ? HIGH : LOW);
  digitalWrite(relayDownPin, relayDownActive ? HIGH : LOW);
}

void TableController::processButtons() {
  unsigned long currentTime = millis();
  
  // Process Up button
  bool rawUpButtonReading = (digitalRead(buttonUpPin) == LOW);
  
  // Debounce logic for Up button
  if (rawUpButtonReading != lastButtonUpState) {
    lastDebounceUpTime = currentTime;
  }
  
  if (currentTime - lastDebounceUpTime > DEBOUNCE_DELAY) {
    bool previousDebouncedState = debouncedButtonUpState;
    debouncedButtonUpState = rawUpButtonReading;
    
    // Detect button press (rising edge: transition from not-pressed to pressed)
    if (debouncedButtonUpState && !previousDebouncedState) {
      // Button pressed
      
      // CRITICAL: If auto-moving, stop immediately on any button press
      if (autoMoving) {
        stop();
        waitingForDoubleClickUp = false;
        pendingMoveUp = false;
        if (DEBUG_SERIAL) {
          Serial.println("⏹️ TableController: Auto movement stopped by up button press");
        }
        // Don't process further for this button press - skip to button release handling
      } else {
        unsigned long timeSinceLastClick = currentTime - lastClickUpTime;
        
        // Check for double-click: second press within timeout
        if (timeSinceLastClick < TABLE_DOUBLE_CLICK_TIMEOUT && waitingForDoubleClickUp) {
          // Double-click detected! Cancel pending move and start auto movement
          waitingForDoubleClickUp = false;
          pendingMoveUp = false;
          moveUpAuto();
          if (DEBUG_SERIAL) {
            Serial.println("🔄 TableController: Double-click up detected - auto moving up");
          }
        } else {
          // First press - schedule delayed start (to allow for double-click detection)
          waitingForDoubleClickUp = true;
          lastClickUpTime = currentTime;
          pendingMoveUp = true;
          pendingMoveUpTime = currentTime;
          if (DEBUG_SERIAL) {
            Serial.println("⏳ TableController: Up button pressed - waiting for double-click or start delay");
          }
        }
      }
    }
    
    // Detect button release (falling edge: transition from pressed to not-pressed)
    if (!debouncedButtonUpState && previousDebouncedState) {
      // Button released - stop only if not auto-moving (manual hold was released)
      if (!autoMoving && relayUpActive) {
        stop();
        pendingMoveUp = false;  // Cancel pending move
        if (DEBUG_SERIAL) {
          Serial.println("🔼 TableController: Up button released - stopped");
        }
      }
      // If pending move not started yet, cancel it
      if (pendingMoveUp && !relayUpActive) {
        pendingMoveUp = false;
        waitingForDoubleClickUp = false;
      }
    }
  }
  
  lastButtonUpState = rawUpButtonReading;
  
  // Process Down button
  bool rawDownButtonReading = (digitalRead(buttonDownPin) == LOW);
  
  // Debounce logic for Down button
  if (rawDownButtonReading != lastButtonDownState) {
    lastDebounceDownTime = currentTime;
  }
  
  if (currentTime - lastDebounceDownTime > DEBOUNCE_DELAY) {
    bool previousDebouncedState = debouncedButtonDownState;
    debouncedButtonDownState = rawDownButtonReading;
    
    // Detect button press (rising edge: transition from not-pressed to pressed)
    if (debouncedButtonDownState && !previousDebouncedState) {
      // Button pressed
      
      // CRITICAL: If auto-moving, stop immediately on any button press
      if (autoMoving) {
        stop();
        waitingForDoubleClickDown = false;
        pendingMoveDown = false;
        if (DEBUG_SERIAL) {
          Serial.println("⏹️ TableController: Auto movement stopped by down button press");
        }
        // Don't process further for this button press - skip to button release handling
      } else {
        unsigned long timeSinceLastClick = currentTime - lastClickDownTime;
        
        // Check for double-click: second press within timeout
        if (timeSinceLastClick < TABLE_DOUBLE_CLICK_TIMEOUT && waitingForDoubleClickDown) {
          // Double-click detected! Cancel pending move and start auto movement
          waitingForDoubleClickDown = false;
          pendingMoveDown = false;
          moveDownAuto();
          if (DEBUG_SERIAL) {
            Serial.println("🔄 TableController: Double-click down detected - auto moving down");
          }
        } else {
          // First press - schedule delayed start (to allow for double-click detection)
          waitingForDoubleClickDown = true;
          lastClickDownTime = currentTime;
          pendingMoveDown = true;
          pendingMoveDownTime = currentTime;
          if (DEBUG_SERIAL) {
            Serial.println("⏳ TableController: Down button pressed - waiting for double-click or start delay");
          }
        }
      }
    }
    
    // Detect button release (falling edge: transition from pressed to not-pressed)
    if (!debouncedButtonDownState && previousDebouncedState) {
      // Button released - stop only if not auto-moving (manual hold was released)
      if (!autoMoving && relayDownActive) {
        stop();
        pendingMoveDown = false;  // Cancel pending move
        if (DEBUG_SERIAL) {
          Serial.println("🔽 TableController: Down button released - stopped");
        }
      }
      // If pending move not started yet, cancel it
      if (pendingMoveDown && !relayDownActive) {
        pendingMoveDown = false;
        waitingForDoubleClickDown = false;
      }
    }
  }
  
  lastButtonDownState = rawDownButtonReading;
  
  // Check auto movement timer
  if (autoMoving) {
    unsigned long elapsed = currentTime - autoMoveStartTime;
    if (elapsed >= TABLE_AUTO_MOVE_DURATION) {
      // Auto movement time expired - stop
      stop();
      if (DEBUG_SERIAL) {
        Serial.println("⏱️ TableController: Auto movement time expired - stopped");
      }
    }
  }
}

void TableController::publishStatus() {
  if (!mqttManager || !mqttManager->isMQTTConnected()) {
    return;  // Cannot publish if MQTT not connected
  }
  
  String direction = getDirection();
  
  // Create JSON payload
  StaticJsonDocument<128> doc;
  doc["direction"] = direction;
  doc["autoMoving"] = autoMoving;  // Indicate if this is auto movement
  
  String payload;
  serializeJson(doc, payload);
  
  // Publish to MQTT
  String topic = MQTT_TOPIC_SENSORS + String("module-4/table/direction");
  bool success = mqttManager->publishRaw(topic, payload);
  
  if (DEBUG_MQTT) {
    if (success) {
      Serial.println("📤 Published table status: " + payload);
    } else {
      Serial.println("❌ Failed to publish table status");
    }
  }
}

String TableController::getDirection() const {
  if (relayUpActive) {
    return "up";
  } else if (relayDownActive) {
    return "down";
  } else {
    return "stopped";
  }
}

void TableController::loop() {
  unsigned long currentTime = millis();
  
  // Process button inputs
  processButtons();
  
  // Check pending moves - start movement after delay if button still pressed and no double-click detected
  if (pendingMoveUp && !relayUpActive && !autoMoving) {
    unsigned long elapsed = currentTime - pendingMoveUpTime;
    // Only start if delay passed AND button is still pressed
    if (elapsed >= TABLE_START_DELAY && debouncedButtonUpState) {
      // Start delay passed and button still held - start movement
      pendingMoveUp = false;
      waitingForDoubleClickUp = false;
      moveUp();
      if (DEBUG_SERIAL) {
        Serial.println("▶️ TableController: Start delay passed - starting up movement");
      }
    } else if (!debouncedButtonUpState) {
      // Button released before delay - cancel pending move
      pendingMoveUp = false;
      waitingForDoubleClickUp = false;
    }
  }
  
  if (pendingMoveDown && !relayDownActive && !autoMoving) {
    unsigned long elapsed = currentTime - pendingMoveDownTime;
    // Only start if delay passed AND button is still pressed
    if (elapsed >= TABLE_START_DELAY && debouncedButtonDownState) {
      // Start delay passed and button still held - start movement
      pendingMoveDown = false;
      waitingForDoubleClickDown = false;
      moveDown();
      if (DEBUG_SERIAL) {
        Serial.println("▶️ TableController: Start delay passed - starting down movement");
      }
    } else if (!debouncedButtonDownState) {
      // Button released before delay - cancel pending move
      pendingMoveDown = false;
      waitingForDoubleClickDown = false;
    }
  }
  
  // Check auto movement timer (handled in processButtons, but also here for safety)
  if (autoMoving) {
    unsigned long elapsed = currentTime - autoMoveStartTime;
    if (elapsed >= TABLE_AUTO_MOVE_DURATION) {
      stop();
    }
  }
  
  // Reset double-click waiting flags if timeout passed (and no pending move)
  if (waitingForDoubleClickUp && !pendingMoveUp && (currentTime - lastClickUpTime) >= TABLE_DOUBLE_CLICK_TIMEOUT) {
    waitingForDoubleClickUp = false;
  }
  if (waitingForDoubleClickDown && !pendingMoveDown && (currentTime - lastClickDownTime) >= TABLE_DOUBLE_CLICK_TIMEOUT) {
    waitingForDoubleClickDown = false;
  }
}

void TableController::forceUpdate() {
  publishStatus();
}

void TableController::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("📊 TableController Status:");
    Serial.println("  Direction: " + getDirection());
    Serial.println("  Relay Up Active: " + String(relayUpActive ? "Yes" : "No"));
    Serial.println("  Relay Down Active: " + String(relayDownActive ? "Yes" : "No"));
    Serial.println("  Auto Moving: " + String(autoMoving ? "Yes" : "No"));
    if (autoMoving) {
      unsigned long elapsed = millis() - autoMoveStartTime;
      Serial.println("  Auto Move Elapsed: " + String(elapsed) + "ms / " + String(TABLE_AUTO_MOVE_DURATION) + "ms");
    }
  }
}

