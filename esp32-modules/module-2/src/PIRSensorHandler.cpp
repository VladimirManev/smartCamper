// PIR Sensor Handler Implementation

#include "PIRSensorHandler.h"
#include "LEDStripController.h"
#include "Config.h"

PIRSensorHandler::PIRSensorHandler(LEDStripController* ledCtrl) 
  : ledController(ledCtrl), lastMotionTime(0), lastPirState(false) {
}

void PIRSensorHandler::begin() {
  if (DEBUG_SERIAL) {
    Serial.println("🏃 PIR Sensor Handler Starting...");
  }
  
  // Initialize PIR sensor pin
  pinMode(PIR_SENSOR_PIN, INPUT);
  
  if (DEBUG_SERIAL) {
    Serial.println("PIR sensor - Pin: " + String(PIR_SENSOR_PIN) + " - OK");
    Serial.println("✅ PIR Sensor Handler Ready!");
  }
}

void PIRSensorHandler::loop() {
  if (!ledController) return;
  
  unsigned long currentTime = millis();
  
  // Get strip state for motion-activated strip
  StripState& motionState = ledController->getStripState(MOTION_STRIP_INDEX);
  
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
        Serial.println("🏃 Motion detected - turning ON strip " + String(MOTION_STRIP_INDEX) + " (Bathroom, pin " + String(PIR_SENSOR_PIN) + ")");
        if (DEBUG_VERBOSE) {
          Serial.println("   Kitchen strip 2 (pin 19) should remain OFF");
        }
        ledController->turnOnStrip(MOTION_STRIP_INDEX);
      } else {
        // Update last motion time
        lastMotionTime = currentTime;
      }
    }
    
    // Check if we need to turn off the strip after timeout
    if (motionState.on && !motionState.transition.active) {
      if (currentTime - lastMotionTime >= PIR_MOTION_TIMEOUT && lastMotionTime > 0) {
        Serial.println("⏱️ Motion timeout (" + String(PIR_MOTION_TIMEOUT / 1000) + "s) - turning OFF strip " + String(MOTION_STRIP_INDEX) + " (Bathroom)");
        ledController->turnOffStrip(MOTION_STRIP_INDEX);
        lastMotionTime = 0;  // Reset
      }
    }
    
    lastPirState = pirState;
  } else {
    // In OFF or ON mode, ignore PIR sensor
    lastPirState = false;
    lastMotionTime = 0;
  }
}

void PIRSensorHandler::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("📊 PIR Sensor Handler Status:");
    Serial.println("  Pin: " + String(PIR_SENSOR_PIN));
    Serial.println("  Current State: " + String(digitalRead(PIR_SENSOR_PIN) == HIGH ? "HIGH" : "LOW"));
    Serial.println("  Last Motion Time: " + String(lastMotionTime > 0 ? String((millis() - lastMotionTime) / 1000) + " seconds ago" : "Never"));
  }
}

