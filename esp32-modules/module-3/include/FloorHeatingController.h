// Floor Heating Controller
// Manages relay control and temperature-based automatic control for heating circles

#ifndef FLOOR_HEATING_CONTROLLER_H
#define FLOOR_HEATING_CONTROLLER_H

#include "Config.h"
#include <Arduino.h>

// Forward declaration
class FloorHeatingSensor;
class FloorHeatingManager;

// CircleMode enum is defined in Config.h

class FloorHeatingController {
private:
  bool relayStates[NUM_HEATING_CIRCLES];
  CircleMode circleModes[NUM_HEATING_CIRCLES];  // OFF or TEMP_CONTROL
  uint8_t relayPins[NUM_HEATING_CIRCLES];
  FloorHeatingSensor* sensors[NUM_HEATING_CIRCLES];  // Pointers to temperature sensors
  FloorHeatingManager* manager;  // Callback to manager for status publishing
  
  // Temperature control settings (future: will be configurable)
  float targetTemperature;  // Default: HEATING_TARGET_TEMP
  float turnOffTemperature; // Default: HEATING_TURN_OFF_TEMP
  float turnOnTemperature;  // Default: HEATING_TURN_ON_TEMP
  
  unsigned long lastControlCheck[NUM_HEATING_CIRCLES];  // Last time we checked temperature for each circle
  
  // Control functions
  void updateCircleControl(uint8_t circleIndex);
  void setRelayState(uint8_t circleIndex, bool state);
  
public:
  FloorHeatingController();
  
  // Initialization
  void begin();
  
  // Set sensor reference for a circle
  void setSensor(uint8_t circleIndex, FloorHeatingSensor* sensor);
  
  // Set manager reference (for status publishing callbacks)
  void setManager(FloorHeatingManager* mgr);
  
  // Main loop - call this in your main loop()
  void loop();
  
  // Control functions
  void setCircleMode(uint8_t circleIndex, CircleMode mode);  // Set circle mode (OFF or TEMP_CONTROL)
  void toggleCircleMode(uint8_t circleIndex);  // Toggle between OFF and TEMP_CONTROL
  
  // Get state
  bool getCircleState(uint8_t circleIndex) const;  // Get relay state (ON/OFF)
  CircleMode getCircleMode(uint8_t circleIndex) const;  // Get circle mode (OFF/TEMP_CONTROL)
  
  // Reset last check time (for immediate temperature check)
  void resetLastCheckTime(uint8_t circleIndex);
  
  // Future: Temperature configuration (for future implementation)
  void setTargetTemperature(float temp);  // Set target temperature (future feature)
  float getTargetTemperature() const { return targetTemperature; }
  
  // Status
  void printStatus() const;
};

#endif

