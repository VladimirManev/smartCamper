#include "AutotermCommands.h"

#include "PacketDecoder.h"

#include <cstring>

size_t AutotermCommands::finishFrame(uint8_t* out, size_t headerAndPayloadLen, size_t outCap) {
  if (out == nullptr || outCap < headerAndPayloadLen + 2) {
    return 0;
  }
  const uint16_t crc = PacketDecoder::crc16Modbus(out, headerAndPayloadLen);
  out[headerAndPayloadLen] = static_cast<uint8_t>((crc >> 8) & 0xFF);
  out[headerAndPayloadLen + 1] = static_cast<uint8_t>(crc & 0xFF);
  return headerAndPayloadLen + 2;
}

size_t AutotermCommands::buildStartByPower(uint8_t power, uint8_t* out, size_t outCap) {
  if (power < 1 || power > 9 || out == nullptr || outCap < 13) {
    return 0;
  }
  const uint8_t body[] = {0xAA, 0x03, 0x06, 0x00, 0x01, 0xFF, 0xFF, 0x04, 0xFF, 0x02, power};
  memcpy(out, body, sizeof(body));
  return finishFrame(out, sizeof(body), outCap);
}

size_t AutotermCommands::buildSetPower(uint8_t power, uint8_t* out, size_t outCap) {
  if (power < 1 || power > 9 || out == nullptr || outCap < 13) {
    return 0;
  }
  const uint8_t body[] = {0xAA, 0x03, 0x06, 0x00, 0x02, 0xFF, 0xFF, 0x04, 0xFF, 0x02, power};
  memcpy(out, body, sizeof(body));
  return finishFrame(out, sizeof(body), outCap);
}

size_t AutotermCommands::buildStop(uint8_t* out, size_t outCap) {
  if (out == nullptr || outCap < 7) {
    return 0;
  }
  const uint8_t body[] = {0xAA, 0x03, 0x00, 0x00, 0x03};
  memcpy(out, body, sizeof(body));
  return finishFrame(out, sizeof(body), outCap);
}
