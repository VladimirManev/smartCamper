// Decode confirmed AUTOTERM frames into a short USB log line.
// Bridge forwarding stays raw; this is display-only.

#ifndef AUTOTERM_PACKET_DECODER_H
#define AUTOTERM_PACKET_DECODER_H

#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>

#include "Config.h"

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

class PacketDecoder {
 public:
  // Writes a short description into text. Returns false if frame is invalid.
  static bool describe(const uint8_t* frame, size_t len, char* text, size_t textCap);
  static uint16_t crc16Modbus(const uint8_t* data, size_t len);
};

#endif
