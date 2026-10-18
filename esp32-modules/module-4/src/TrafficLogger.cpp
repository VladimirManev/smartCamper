#include "TrafficLogger.h"

namespace {

void printTimestamp(uint32_t timeUs) {
  const uint32_t ms = timeUs / 1000UL;
  const uint32_t frac = timeUs % 1000UL;
  Serial.printf("[%10lu.%03lu] ", static_cast<unsigned long>(ms), static_cast<unsigned long>(frac));
}

void printHexWrapped(const uint8_t* data, size_t len) {
  for (size_t i = 0; i < len; ++i) {
    if (i > 0 && (i % 16) == 0) {
      Serial.print("\n          ");
    }
    Serial.printf("%02X ", data[i]);
  }
}

}  // namespace

void TrafficLogger::logDecoded(const char* direction, uint32_t firstByteUs, const char* text) {
  if (direction == nullptr || text == nullptr) {
    return;
  }
  printTimestamp(firstByteUs);
  Serial.printf("%s: %s\n", direction, text);
}

void TrafficLogger::logBurst(const char* direction, uint32_t firstByteUs, const uint8_t* data, size_t len, bool split) {
  if (data == nullptr || len == 0 || direction == nullptr) {
    return;
  }

  printTimestamp(firstByteUs);
  Serial.printf("%s (%u)%s: ", direction, static_cast<unsigned>(len), split ? " split" : "");
  printHexWrapped(data, len);
  Serial.println();
}

void TrafficLogger::logByte(const char* direction, uint32_t byteUs, uint32_t deltaUs, uint8_t value, bool deltaValid) {
  if (direction == nullptr) {
    return;
  }

  printTimestamp(byteUs);
  if (deltaValid) {
    Serial.printf("%s +%lu us  %02X\n", direction, static_cast<unsigned long>(deltaUs), value);
  } else {
    Serial.printf("%s  %02X\n", direction, value);
  }
}
