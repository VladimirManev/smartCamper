// GPIO checks for classic ESP32 (esp32dev). Rejects pins that would steal
// the USB log, hit the flash bus, or prevent boot.

#ifndef AUTOTERM_ESP32_PINS_H
#define AUTOTERM_ESP32_PINS_H

#include <Arduino.h>

struct PinCheck {
  bool ok;
  bool warning;
  const char* message;
};

inline PinCheck checkRxPin(int8_t pin) {
  if (pin < 0 || pin > 39) {
    return {false, false, "GPIO out of range (0-39)"};
  }
  if (pin >= 6 && pin <= 11) {
    return {false, false, "GPIO 6-11 are tied to flash"};
  }
  if (pin == 1 || pin == 3) {
    return {false, false, "GPIO 1 and 3 are the USB log port"};
  }
  if (pin == 12) {
    return {false, false, "GPIO 12 is the flash-voltage strap pin"};
  }
  if (pin == 20 || pin == 24 || (pin >= 28 && pin <= 31)) {
    return {false, false, "this GPIO does not exist on ESP32"};
  }
  if (pin == 0 || pin == 2 || pin == 5 || pin == 15) {
    return {true, true, "strapping pin — keep it high while the ESP32 boots"};
  }
  return {true, false, nullptr};
}

inline PinCheck checkReservedTxPin(int8_t pin) {
  PinCheck rx = checkRxPin(pin);
  if (!rx.ok) {
    return rx;
  }
  if (pin >= 34 && pin <= 39) {
    return {false, false, "GPIO 34-39 are input-only and cannot be TX later"};
  }
  return rx;
}

#endif
