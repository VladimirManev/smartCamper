// One HardwareSerial port for Autoterm bridge (RX from device, TX toward device).

#ifndef AUTOTERM_UART_ENDPOINT_H
#define AUTOTERM_UART_ENDPOINT_H

#include <Arduino.h>

class UartEndpoint {
 public:
  UartEndpoint(HardwareSerial& serial, const char* name);

  bool start(int8_t rxPin, int8_t txPin, uint32_t baud, uint32_t serialConfig, bool invert, bool rxPullup);
  bool restart();

  int available();
  int readByte();
  size_t writeByte(uint8_t value);
  size_t write(const uint8_t* data, size_t len);

  const char* name() const { return name_; }
  int8_t rxPin() const { return rxPin_; }
  int8_t txPin() const { return txPin_; }
  bool isOpen() const { return open_; }

 private:
  void applyRxPullup() const;

  HardwareSerial& serial_;
  const char* name_;
  int8_t rxPin_;
  int8_t txPin_;
  uint32_t baud_;
  uint32_t serialConfig_;
  bool invert_;
  bool rxPullup_;
  bool open_;
};

#endif
