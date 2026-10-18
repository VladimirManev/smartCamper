// One HardwareSerial port: RX from AUTOTERM, TX toward AUTOTERM.
// Stage 2 drives TX and forwards bytes unchanged.

#ifndef AUTOTERM_UART_ENDPOINT_H
#define AUTOTERM_UART_ENDPOINT_H

#include <Arduino.h>

class UartEndpoint {
 public:
  UartEndpoint(HardwareSerial& serial, const char* name, const char* direction);

  bool start(int8_t rxPin, int8_t txPin, uint32_t baud, uint32_t serialConfig, bool invert, bool rxPullup);
  bool restart();

  int available();
  int readByte();
  size_t writeByte(uint8_t value);
  size_t write(const uint8_t* data, size_t len);

  const char* name() const { return name_; }
  const char* direction() const { return direction_; }
  int8_t rxPin() const { return rxPin_; }
  int8_t txPin() const { return txPin_; }
  uint32_t baud() const { return baud_; }
  uint32_t serialConfig() const { return serialConfig_; }
  bool invert() const { return invert_; }
  bool isOpen() const { return open_; }

  bool setRxPin(int8_t rxPin);
  bool setBaud(uint32_t baud);
  bool setSerialConfig(uint32_t serialConfig);
  bool setInvert(bool invert);

 private:
  void applyRxPullup() const;

  HardwareSerial& serial_;
  const char* name_;
  const char* direction_;
  int8_t rxPin_;
  int8_t txPin_;
  uint32_t baud_;
  uint32_t serialConfig_;
  bool invert_;
  bool rxPullup_;
  bool open_;
};

const char* serialFormatName(uint32_t config);
bool parseSerialFormat(const char* text, uint32_t& config);

#endif
