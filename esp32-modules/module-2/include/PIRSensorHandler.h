// PIR Sensor Handler
// Handles PIR motion sensor for automatic strip control (Bathroom - Strip 3)

#ifndef PIR_SENSOR_HANDLER_H
#define PIR_SENSOR_HANDLER_H

#include "Config.h"
#include "StripState.h"
#include <Arduino.h>

// Forward declaration
class LEDStripController;

class PIRSensorHandler {
private:
  LEDStripController* ledController;
  
  unsigned long lastMotionTime;
  bool lastPirState;
  
public:
  PIRSensorHandler(LEDStripController* ledCtrl);
  
  // Initialization
  void begin();
  
  // Main loop - call this in your main loop()
  void loop();
  
  // Status
  void printStatus() const;
};

#endif

