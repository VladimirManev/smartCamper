// Build confirmed panel->heater frames for by-power control.

#ifndef AUTOTERM_COMMANDS_H
#define AUTOTERM_COMMANDS_H

#include <stddef.h>
#include <stdint.h>

class AutotermCommands {
 public:
  // wirePower: Autoterm protocol 0..9 (UI levels are 1..10 = wire+1).
  static size_t buildStartByPower(uint8_t wirePower, uint8_t* out, size_t outCap);
  static size_t buildSetPower(uint8_t wirePower, uint8_t* out, size_t outCap);
  static size_t buildStop(uint8_t* out, size_t outCap);

 private:
  static size_t finishFrame(uint8_t* out, size_t headerAndPayloadLen, size_t outCap);
};

#endif
