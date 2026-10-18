// Build confirmed panel->heater frames for injection tests.
// Only frame types captured on this Comfort Control + Air 2D pair.

#ifndef AUTOTERM_COMMANDS_H
#define AUTOTERM_COMMANDS_H

#include <stddef.h>
#include <stdint.h>

class AutotermCommands {
 public:
  // START heat, mode by-power, vent off, power 1..9. Returns frame length (13).
  static size_t buildStartByPower(uint8_t power, uint8_t* out, size_t outCap);

  // Change settings while running: by-power, vent off, power 1..9. Returns 13.
  static size_t buildSetPower(uint8_t power, uint8_t* out, size_t outCap);

  // Ventilation only, level 1..9. Returns 11.
  // Air 2D form: AA 03 04 00 23 FF FF <level> FF <crc>
  static size_t buildVent(uint8_t level, uint8_t* out, size_t outCap);

  // STOP. Returns 7.
  static size_t buildStop(uint8_t* out, size_t outCap);

 private:
  static size_t finishFrame(uint8_t* out, size_t headerAndPayloadLen, size_t outCap);
};

#endif
