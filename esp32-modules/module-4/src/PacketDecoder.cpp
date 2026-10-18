#include "PacketDecoder.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace {

bool appendf(char*& cursor, char* end, const char* fmt, ...) {
  if (cursor >= end) {
    return false;
  }
  va_list args;
  va_start(args, fmt);
  const int n = vsnprintf(cursor, static_cast<size_t>(end - cursor), fmt, args);
  va_end(args);
  if (n < 0 || n >= static_cast<int>(end - cursor)) {
    if (end > cursor) {
      *(end - 1) = '\0';
    }
    return false;
  }
  cursor += n;
  return true;
}

bool appendPart(char* text, char*& cursor, char* end, const char* fmt, ...) {
  if (cursor >= end) {
    return false;
  }

  char token[48];
  va_list args;
  va_start(args, fmt);
  const int n = vsnprintf(token, sizeof(token), fmt, args);
  va_end(args);
  if (n <= 0) {
    return false;
  }

  if (text[0] != '\0' && !appendf(cursor, end, " ")) {
    return false;
  }
  return appendf(cursor, end, "%s", token);
}

const char* modeWord(uint8_t mode) {
  switch (mode) {
    case 0x01:
      return "by-heater-T";
    case 0x02:
      return "by-panel-T";
    case 0x03:
      return "by-air-T";
    case 0x04:
      return "by-power";
    default:
      return "mode?";
  }
}

const char* statusWord(uint8_t s0, uint8_t s1) {
  if (s0 == 0x00 && s1 == 0x01) {
    return "off";
  }
  if (s0 == 0x01) {
    return "starting";
  }
  if (s0 == 0x02) {
    return "running";
  }
  if (s0 == 0x03 && s1 == 0x23) {
    return "vent";
  }
  if (s0 == 0x03) {
    return "running?";
  }
  if (s0 == 0x04) {
    return "shutdown";
  }
  return "state?";
}

void describeSettingsPayload(char* text, char*& cursor, char* end, const uint8_t* p, size_t n) {
  if (n < 6) {
    appendPart(text, cursor, end, "settings(%u)", static_cast<unsigned>(n));
    return;
  }
  const uint8_t mode = p[2];
  const uint8_t temp = p[3];
  const uint8_t vent = p[4];
  const uint8_t power = p[5];
  appendPart(text, cursor, end, "%s", modeWord(mode));
  if (temp != 0xFF) {
    appendPart(text, cursor, end, "set=%uC", temp);
  }
  if (vent == 0x01) {
    appendPart(text, cursor, end, "vent=on");
  } else if (vent == 0x02) {
    appendPart(text, cursor, end, "vent=off");
  }
  if (power != 0xFF) {
    appendPart(text, cursor, end, "pwr=%u", power);
  }
}

void describeStatusPayload(char* text, char*& cursor, char* end, const uint8_t* p, size_t n) {
  if (n < 7) {
    appendPart(text, cursor, end, "status(%u)", static_cast<unsigned>(n));
    return;
  }
  appendPart(text, cursor, end, "%s", statusWord(p[0], p[1]));
  if (p[2] != 0) {
    appendPart(text, cursor, end, "err=%u", p[2]);
  }
  appendPart(text, cursor, end, "Ht=%uC", p[3]);
  if (p[4] == 0x7F) {
    appendPart(text, cursor, end, "Ext=-");
  } else {
    appendPart(text, cursor, end, "Ext=%uC", p[4]);
  }
  if (n > 6) {
    appendPart(text, cursor, end, "Bat=%u.%uV", p[6] / 10, p[6] % 10);
  }
}

}  // namespace

void FrameAssembler::reset() {
  len_ = 0;
  need_ = 0;
  firstUs_ = 0;
  inFrame_ = false;
}

bool FrameAssembler::feed(uint8_t byte, uint8_t* out, size_t outCap, size_t& outLen, uint32_t nowUs, uint32_t& firstUs) {
  outLen = 0;
  if (out == nullptr || outCap < 7) {
    return false;
  }

  if (!inFrame_) {
    if (byte != 0xAA) {
      return false;
    }
    inFrame_ = true;
    len_ = 0;
    need_ = 5;
    firstUs_ = nowUs;
    buf_[len_++] = byte;
    return false;
  }

  if (len_ >= sizeof(buf_)) {
    reset();
    return false;
  }
  buf_[len_++] = byte;

  if (len_ == 3) {
    need_ = 5 + buf_[2] + 2;
    if (need_ > sizeof(buf_)) {
      reset();
      return false;
    }
  }

  if (len_ >= 3 && len_ >= need_) {
    if (need_ > outCap) {
      reset();
      return false;
    }
    memcpy(out, buf_, need_);
    outLen = need_;
    firstUs = firstUs_;
    reset();
    return true;
  }
  return false;
}

uint16_t PacketDecoder::crc16Modbus(const uint8_t* data, size_t len) {
  uint16_t crc = 0xFFFF;
  for (size_t pos = 0; pos < len; ++pos) {
    crc = static_cast<uint16_t>(crc ^ data[pos]);
    for (int i = 0; i < 8; ++i) {
      if (crc & 0x0001U) {
        crc = static_cast<uint16_t>((crc >> 1) ^ 0xA001U);
      } else {
        crc = static_cast<uint16_t>(crc >> 1);
      }
    }
  }
  return crc;
}

const char* PacketDecoder::errorMessage(uint8_t errorCode) {
  // Common Autoterm / Planar service codes (Air 2D family). Unknown → generic.
  switch (errorCode) {
    case 0:
      return nullptr;
    case 1:
      return "Heat exchanger overheat";
    case 2:
      return "Case overheat";
    case 3:
      return "Start failure";
    case 4:
      return "Output short circuit";
    case 5:
      return "Flame failure";
    case 6:
      return "No ignition / no fuel";
    case 7:
      return "Runtime fault";
    case 8:
      return "Heat exchanger sensor";
    case 9:
      return "Glow plug";
    case 10:
      return "Blower / fan";
    case 11:
      return "Communication";
    case 12:
      return "Battery overvoltage";
    case 13:
      return "Battery undervoltage";
    case 14:
      return "Overheat";
    case 15:
      return "Fuel pump";
    case 16:
      return "Controller";
    case 17:
      return "Flame sensor";
    default:
      return "Unknown fault";
  }
}

bool PacketDecoder::parseStatusReply(const uint8_t* frame, size_t len, AutotermParsedStatus& out) {
  out = {false, "unknown", 0, -127, -127, -1.0f};
  if (frame == nullptr || len < 7 || frame[0] != 0xAA || frame[4] != 0x0F) {
    return false;
  }
  const uint8_t payloadLen = frame[2];
  const size_t expected = 5UL + payloadLen + 2UL;
  if (len != expected || payloadLen < 7) {
    return false;
  }
  const uint16_t got = static_cast<uint16_t>((frame[len - 2] << 8) | frame[len - 1]);
  if (got != crc16Modbus(frame, len - 2)) {
    return false;
  }
  const uint8_t* p = frame + 5;
  out.state = statusWord(p[0], p[1]);
  out.errorCode = p[2];
  out.heaterTempC = static_cast<int>(p[3]);
  if (p[4] == 0x7F) {
    out.extTempC = -127;
  } else {
    out.extTempC = static_cast<int>(p[4]);
  }
  out.batteryV = static_cast<float>(p[6]) / 10.0f;
  out.valid = true;
  return true;
}

bool PacketDecoder::parseSettingsReply(const uint8_t* frame, size_t len, AutotermParsedSettings& out) {
  out = {false, "unknown", 0, 0xFF};
  if (frame == nullptr || len < 7 || frame[0] != 0xAA || frame[4] != 0x02) {
    return false;
  }
  const uint8_t payloadLen = frame[2];
  const size_t expected = 5UL + payloadLen + 2UL;
  if (len != expected || payloadLen < 6) {
    return false;
  }
  const uint16_t got = static_cast<uint16_t>((frame[len - 2] << 8) | frame[len - 1]);
  if (got != crc16Modbus(frame, len - 2)) {
    return false;
  }
  const uint8_t* p = frame + 5;
  out.mode = modeWord(p[2]);
  out.setTempC = p[3];
  out.power = (p[5] != 0xFF) ? p[5] : 0;
  out.valid = true;
  return true;
}

bool PacketDecoder::describe(const uint8_t* frame, size_t len, char* text, size_t textCap) {
  if (frame == nullptr || text == nullptr || textCap < 8 || len < 7) {
    return false;
  }
  if (frame[0] != 0xAA) {
    return false;
  }

  const uint8_t payloadLen = frame[2];
  const size_t expected = 5UL + payloadLen + 2UL;
  if (len != expected || frame[3] != 0x00) {
    snprintf(text, textCap, "bad frame len=%u", static_cast<unsigned>(len));
    return true;
  }

  const uint16_t got = static_cast<uint16_t>((frame[len - 2] << 8) | frame[len - 1]);
  const uint16_t calc = crc16Modbus(frame, len - 2);
  const bool crcOk = (got == calc);
  const uint8_t cmd = frame[4];
  const uint8_t* payload = frame + 5;

  char* cursor = text;
  char* end = text + textCap;
  text[0] = '\0';

  switch (cmd) {
    case 0x0F:
      if (payloadLen == 0) {
        appendPart(text, cursor, end, "status?");
      } else {
        describeStatusPayload(text, cursor, end, payload, payloadLen);
      }
      break;
    case 0x11:
      if (payloadLen >= 1) {
        appendPart(text, cursor, end, "temp %uC", payload[0]);
      } else {
        appendPart(text, cursor, end, "temp?");
      }
      break;
    case 0x02:
      if (payloadLen == 0) {
        appendPart(text, cursor, end, "settings?");
      } else {
        describeSettingsPayload(text, cursor, end, payload, payloadLen);
      }
      break;
    case 0x01:
      appendPart(text, cursor, end, "START");
      describeSettingsPayload(text, cursor, end, payload, payloadLen);
      break;
    case 0x03:
      appendPart(text, cursor, end, "STOP");
      break;
    case 0x23:
      appendPart(text, cursor, end, "VENT");
      // Air 2D: FF FF <level> FF  — or panel variant 00 <n> 00
      if (payloadLen >= 3 && payload[0] == 0xFF && payload[1] == 0xFF) {
        appendPart(text, cursor, end, "%u", payload[2]);
      } else if (payloadLen >= 2 && payload[1] != 0) {
        appendPart(text, cursor, end, "%u", payload[1]);
      }
      break;
    case 0x04:
      appendPart(text, cursor, end, "init");
      break;
    case 0x06:
      appendPart(text, cursor, end, "version");
      break;
    default:
      appendPart(text, cursor, end, "cmd=0x%02X len=%u", cmd, payloadLen);
      break;
  }

  if (!crcOk) {
    appendPart(text, cursor, end, "CRC?");
  }
  return true;
}
