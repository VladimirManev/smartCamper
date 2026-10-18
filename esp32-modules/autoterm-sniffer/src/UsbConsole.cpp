#include "UsbConsole.h"

#include "Config.h"
#include "UartEndpoint.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

namespace {

void lowerInPlace(char* text) {
  if (text == nullptr) {
    return;
  }
  for (char* p = text; *p != '\0'; ++p) {
    *p = static_cast<char>(tolower(static_cast<unsigned char>(*p)));
  }
}

int splitArgs(char* line, char* argv[], int maxArgs) {
  int argc = 0;
  char* cursor = line;
  while (*cursor != '\0' && argc < maxArgs) {
    while (*cursor == ' ' || *cursor == '\t') {
      ++cursor;
    }
    if (*cursor == '\0') {
      break;
    }
    argv[argc++] = cursor;
    while (*cursor != '\0' && *cursor != ' ' && *cursor != '\t') {
      ++cursor;
    }
    if (*cursor != '\0') {
      *cursor = '\0';
      ++cursor;
    }
  }
  return argc;
}

bool parseU32(const char* text, uint32_t& value) {
  if (text == nullptr || *text == '\0') {
    return false;
  }
  char* end = nullptr;
  const unsigned long parsed = strtoul(text, &end, 10);
  if (end == text || *end != '\0') {
    return false;
  }
  value = static_cast<uint32_t>(parsed);
  return true;
}

bool parsePort(const char* text, PortId& port) {
  if (text == nullptr) {
    return false;
  }
  if (strcmp(text, "panel") == 0 || strcmp(text, "uart1") == 0 || strcmp(text, "1") == 0) {
    port = PortId::Panel;
    return true;
  }
  if (strcmp(text, "heater") == 0 || strcmp(text, "uart2") == 0 || strcmp(text, "2") == 0) {
    port = PortId::Heater;
    return true;
  }
  return false;
}

}  // namespace

UsbConsole::UsbConsole(Sniffer& sniffer) : sniffer_(sniffer), length_(0), overflow_(false) {
  line_[0] = '\0';
}

void UsbConsole::begin() {
  printHelp();
}

void UsbConsole::poll() {
  while (Serial.available() > 0) {
    const int raw = Serial.read();
    if (raw < 0) {
      return;
    }
    const char ch = static_cast<char>(raw);
    if (ch == '\r') {
      continue;
    }
    if (overflow_) {
      if (ch == '\n') {
        overflow_ = false;
      }
      continue;
    }
    if (ch == '\n') {
      line_[length_] = '\0';
      length_ = 0;
      if (line_[0] != '\0') {
        handleLine(line_);
      }
      continue;
    }
    if (length_ + 1 >= sizeof(line_)) {
      length_ = 0;
      line_[0] = '\0';
      overflow_ = true;
      Serial.println("line too long");
      continue;
    }
    line_[length_++] = ch;
  }
}

void UsbConsole::printHelp() const {
  Serial.println("commands (USB console only — does not inject AUTOTERM commands):");
  Serial.println("  help");
  Serial.println("  status");
  Serial.println("  pins");
  Serial.println("  reset");
  Serial.println("  baud <rate>                 both ports");
  Serial.println("  baud panel|heater <rate>");
  Serial.println("  serial 8n1|8e1|8o1|8n2      both ports");
  Serial.println("  serial panel|heater <fmt>");
  Serial.println("  invert on|off");
  Serial.println("  gap <ms>                    hex log grouping, 1..500");
  Serial.println("  mode decode|hex|bytes       default: decode");
  Serial.println("  rx panel|heater <gpio>");
}

void UsbConsole::handleLine(char* line) {
  char* argv[4] = {};
  const int argc = splitArgs(line, argv, 4);
  if (argc <= 0) {
    return;
  }
  lowerInPlace(argv[0]);

  if (strcmp(argv[0], "help") == 0) {
    printHelp();
    return;
  }
  if (strcmp(argv[0], "status") == 0) {
    sniffer_.printStatus();
    return;
  }
  if (strcmp(argv[0], "pins") == 0) {
    sniffer_.printPins();
    return;
  }
  if (strcmp(argv[0], "reset") == 0) {
    sniffer_.resetStats();
    return;
  }
  if (strcmp(argv[0], "mode") == 0 && argc == 2) {
    lowerInPlace(argv[1]);
    if (strcmp(argv[1], "decode") == 0) {
      sniffer_.setLogMode(LogMode::Decode);
      return;
    }
    if (strcmp(argv[1], "hex") == 0 || strcmp(argv[1], "burst") == 0) {
      sniffer_.setLogMode(LogMode::Hex);
      return;
    }
    if (strcmp(argv[1], "bytes") == 0) {
      sniffer_.setLogMode(LogMode::Bytes);
      return;
    }
  }
  if (strcmp(argv[0], "gap") == 0 && argc == 2) {
    uint32_t gap = 0;
    if (!parseU32(argv[1], gap) || gap > 65535UL) {
      Serial.println("usage: gap <ms>");
      return;
    }
    sniffer_.setGapMs(static_cast<uint16_t>(gap));
    return;
  }
  if (strcmp(argv[0], "invert") == 0 && argc == 2) {
    lowerInPlace(argv[1]);
    if (strcmp(argv[1], "on") == 0) {
      sniffer_.setInvert(true);
      return;
    }
    if (strcmp(argv[1], "off") == 0) {
      sniffer_.setInvert(false);
      return;
    }
  }
  if (strcmp(argv[0], "baud") == 0 && argc == 2) {
    uint32_t baud = 0;
    if (!parseU32(argv[1], baud)) {
      Serial.println("usage: baud <rate>");
      return;
    }
    sniffer_.setBaud(baud);
    return;
  }
  if (strcmp(argv[0], "baud") == 0 && argc == 3) {
    lowerInPlace(argv[1]);
    PortId port;
    uint32_t baud = 0;
    if (!parsePort(argv[1], port) || !parseU32(argv[2], baud)) {
      Serial.println("usage: baud panel|heater <rate>");
      return;
    }
    sniffer_.setBaud(port, baud);
    return;
  }
  if (strcmp(argv[0], "serial") == 0 && argc == 2) {
    lowerInPlace(argv[1]);
    uint32_t config = 0;
    if (!parseSerialFormat(argv[1], config)) {
      Serial.println("usage: serial 8n1|8e1|8o1|8n2");
      return;
    }
    sniffer_.setFormat(config);
    return;
  }
  if (strcmp(argv[0], "serial") == 0 && argc == 3) {
    lowerInPlace(argv[1]);
    lowerInPlace(argv[2]);
    PortId port;
    uint32_t config = 0;
    if (!parsePort(argv[1], port) || !parseSerialFormat(argv[2], config)) {
      Serial.println("usage: serial panel|heater 8n1|8e1|8o1|8n2");
      return;
    }
    sniffer_.setFormat(port, config);
    return;
  }
  if (strcmp(argv[0], "rx") == 0 && argc == 3) {
    lowerInPlace(argv[1]);
    PortId port;
    uint32_t gpio = 0;
    if (!parsePort(argv[1], port) || !parseU32(argv[2], gpio) || gpio > 39UL) {
      Serial.println("usage: rx panel|heater <gpio>");
      return;
    }
    sniffer_.setRxPin(port, static_cast<int8_t>(gpio));
    return;
  }

  Serial.println("unknown command, type help");
}
