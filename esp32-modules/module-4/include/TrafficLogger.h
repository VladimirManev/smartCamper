// USB log lines for bridge traffic.

#ifndef AUTOTERM_TRAFFIC_LOGGER_H
#define AUTOTERM_TRAFFIC_LOGGER_H

#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>

class TrafficLogger {
 public:
  static void logDecoded(const char* direction, uint32_t firstByteUs, const char* text);
  static void logBurst(const char* direction, uint32_t firstByteUs, const uint8_t* data, size_t len, bool split);
  static void logByte(const char* direction, uint32_t byteUs, uint32_t deltaUs, uint8_t value, bool deltaValid);
};

#endif
