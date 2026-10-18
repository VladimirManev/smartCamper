// Decode confirmed AUTOTERM frames into a short USB log line.
// Bridge forwarding stays raw; this is display-only.

#ifndef AUTOTERM_PACKET_DECODER_H
#define AUTOTERM_PACKET_DECODER_H

#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>

class FrameAssembler {
 public:
  void reset();
  // Returns true when one complete frame was assembled into out[].
  bool feed(uint8_t byte, uint8_t* out, size_t outCap, size_t& outLen, uint32_t nowUs, uint32_t& firstUs);

 private:
  uint8_t buf_[5 + 255 + 2];
  size_t len_;
  size_t need_;
  uint32_t firstUs_;
  bool inFrame_;
};

struct AutotermParsedStatus {
  bool valid;
  const char* state;  // off, starting, running, vent, shutdown, unknown
  uint8_t errorCode;  // 0 = none (status payload byte 2)
  int heaterTempC;    // -127 if missing
  int extTempC;       // -127 if missing / none
  float batteryV;     // -1 if missing
};

struct AutotermParsedSettings {
  bool valid;
  const char* mode;  // by-heater-T, by-panel-T, by-air-T, by-power, unknown
  uint8_t power;     // 0 if unknown
  uint8_t setTempC;  // 0xFF if n/a
};

class PacketDecoder {
 public:
  // Writes a short description into text. Returns false if frame is invalid.
  static bool describe(const uint8_t* frame, size_t len, char* text, size_t textCap);
  static uint16_t crc16Modbus(const uint8_t* data, size_t len);

  // Parse heater replies (cmd 0x0F status payload / 0x02 settings payload).
  static bool parseStatusReply(const uint8_t* frame, size_t len, AutotermParsedStatus& out);
  static bool parseSettingsReply(const uint8_t* frame, size_t len, AutotermParsedSettings& out);

  // Short English label for Autoterm/Planar fault codes (payload error byte).
  static const char* errorMessage(uint8_t errorCode);
};

#endif
