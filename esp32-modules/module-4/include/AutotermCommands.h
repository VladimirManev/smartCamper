// Build confirmed panel->heater frames for by-power control.

#ifndef AUTOTERM_COMMANDS_H
#define AUTOTERM_COMMANDS_H

#include <stddef.h>
#include <stdint.h>

class AutotermCommands {
 public:
  static size_t buildStartByPower(uint8_t power, uint8_t* out, size_t outCap);
  static size_t buildSetPower(uint8_t power, uint8_t* out, size_t outCap);
  static size_t buildStop(uint8_t* out, size_t outCap);

 private:
  static size_t finishFrame(uint8_t* out, size_t headerAndPayloadLen, size_t outCap);
};

#endif
