// Transparent bridge: bytes from one UART are written to the other unchanged.
// USB log can decode frames into short text (default).

#ifndef AUTOTERM_SNIFFER_H
#define AUTOTERM_SNIFFER_H

#include "Config.h"
#include "PacketDecoder.h"
#include "UartEndpoint.h"

struct BurstBuffer {
  uint8_t data[kBurstBytes];
  size_t len;
  uint32_t firstUs;
  uint32_t lastUs;
  bool active;
};

struct PortStats {
  uint32_t bytesRx;
  uint32_t bytesTx;
  uint32_t frames;
  uint32_t splitFlushes;
  uint32_t lastByteUs;
  bool hasByte;
  int peakAvailable;
};

class Sniffer {
 public:
  Sniffer(HardwareSerial& panelSerial, HardwareSerial& heaterSerial);

  void begin();
  void poll();

  bool setBaud(uint32_t baud);
  bool setBaud(PortId port, uint32_t baud);
  bool setFormat(uint32_t serialConfig);
  bool setFormat(PortId port, uint32_t serialConfig);
  bool setInvert(bool invert);
  bool setGapMs(uint16_t gapMs);
  bool setRxPin(PortId port, int8_t pin);
  void setLogMode(LogMode mode);
  void resetStats();

  void printStatus() const;
  void printPins() const;

  // Write a panel-format frame to the heater UART (injection). Bridge stays active.
  bool injectToHeater(const uint8_t* frame, size_t len);

  uint16_t gapMs() const { return gapMs_; }
  LogMode logMode() const { return logMode_; }

 private:
  void pollOne(UartEndpoint& source,
               UartEndpoint& peer,
               FrameAssembler& assembler,
               BurstBuffer& burst,
               PortStats& rxStats,
               PortStats& txStats);
  void flushBurst(UartEndpoint& port, BurstBuffer& burst, PortStats& stats, bool split);
  void flushOpenBursts();
  void resetAssemblers();
  static const char* logModeName(LogMode mode);

  UartEndpoint panel_;
  UartEndpoint heater_;
  FrameAssembler panelAssembler_;
  FrameAssembler heaterAssembler_;
  BurstBuffer panelBurst_;
  BurstBuffer heaterBurst_;
  PortStats panelStats_;
  PortStats heaterStats_;
  uint32_t gapUs_;
  uint16_t gapMs_;
  LogMode logMode_;
  bool rxPullup_;
};

#endif
