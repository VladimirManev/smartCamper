#ifndef CAN_DOOR_READER_H
#define CAN_DOOR_READER_H

#include <Arduino.h>

struct CanDoorState {
  bool driver;
  bool passenger;
  bool sliding;
  bool rear;
};

// Fiat Ducato B-CAN door status (listen-only TWAI).
class CanDoorReader {
public:
  CanDoorReader();

  void begin();
  void loop();

  bool isReady() const { return ready; }
  CanDoorState getState() const { return state; }

  /** True once after door bits change; clears on call. */
  bool consumeChanged();

private:
  bool ready;
  bool haveState;
  bool changed;
  CanDoorState state;

  void handleFrame(uint32_t id, const uint8_t *data, uint8_t dlc);
};

#endif
