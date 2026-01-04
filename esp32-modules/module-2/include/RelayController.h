// Relay Controller
// Manages relay control

#ifndef RELAY_CONTROLLER_H
#define RELAY_CONTROLLER_H

#include "Config.h"
#include <Arduino.h>

class RelayController {
private:
  bool relayStates[NUM_RELAYS];
  uint8_t relayPins[NUM_RELAYS] = {RELAY_PIN_0};
  
public:
  RelayController();
  
  // Initialization
  void begin();
  
  // Control functions
  void toggleRelay(uint8_t relayIndex);
  void setRelay(uint8_t relayIndex, bool state);
  bool getRelayState(uint8_t relayIndex) const;
  
  // Get all relay states (for status publishing)
  const bool* getRelayStates() const { return relayStates; }
  
  // Status
  void printStatus() const;
};

#endif

