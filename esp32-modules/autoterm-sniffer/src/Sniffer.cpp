#include "Sniffer.h"

#include "Config.h"
#include "TrafficLogger.h"

namespace {

void clearBurst(BurstBuffer& burst) {
  burst.len = 0;
  burst.firstUs = 0;
  burst.lastUs = 0;
  burst.active = false;
}

bool pinsAreDistinct(int8_t a, int8_t b, int8_t c, int8_t d) {
  const int8_t pins[] = {a, b, c, d};
  for (int i = 0; i < 4; ++i) {
    for (int j = i + 1; j < 4; ++j) {
      if (pins[i] == pins[j]) {
        return false;
      }
    }
  }
  return true;
}

void clearStats(PortStats& stats) {
  stats.bytesRx = 0;
  stats.bytesTx = 0;
  stats.frames = 0;
  stats.splitFlushes = 0;
  stats.lastByteUs = 0;
  stats.hasByte = false;
  stats.peakAvailable = 0;
}

void printPortStatus(const UartEndpoint& port, const PortStats& stats) {
  Serial.printf("  %s  RX GPIO %d   TX GPIO %d   %lu %s%s\n",
                port.name(),
                port.rxPin(),
                port.txPin(),
                static_cast<unsigned long>(port.baud()),
                serialFormatName(port.serialConfig()),
                port.invert() ? " inverted" : "");
  Serial.printf("    rx %lu   tx %lu   frames %lu   splits %lu   rx peak %d",
                static_cast<unsigned long>(stats.bytesRx),
                static_cast<unsigned long>(stats.bytesTx),
                static_cast<unsigned long>(stats.frames),
                static_cast<unsigned long>(stats.splitFlushes),
                stats.peakAvailable);
  if (stats.hasByte) {
    const uint32_t ageMs = (micros() - stats.lastByteUs) / 1000UL;
    Serial.printf("   last byte %lu ms ago", static_cast<unsigned long>(ageMs));
  } else {
    Serial.print("   no bytes yet");
  }
  Serial.println();
  if (stats.peakAvailable > static_cast<int>(kRxBufferBytes * 3 / 4)) {
    Serial.println("    WARNING: RX buffer peaked above 75%. Bytes may have been dropped.");
  }
}

}  // namespace

Sniffer::Sniffer(HardwareSerial& panelSerial, HardwareSerial& heaterSerial)
    : panel_(panelSerial, "PANEL", "PANEL"),
      heater_(heaterSerial, "HEATER", "HEATER"),
      gapUs_(static_cast<uint32_t>(kFrameGapMs) * 1000UL),
      gapMs_(kFrameGapMs),
      logMode_(LogMode::Decode),
      rxPullup_(kRxPullup) {
  clearBurst(panelBurst_);
  clearBurst(heaterBurst_);
  clearStats(panelStats_);
  clearStats(heaterStats_);
  panelAssembler_.reset();
  heaterAssembler_.reset();
}

const char* Sniffer::logModeName(LogMode mode) {
  switch (mode) {
    case LogMode::Decode:
      return "decode";
    case LogMode::Hex:
      return "hex";
    case LogMode::Bytes:
      return "bytes";
  }
  return "?";
}

void Sniffer::begin() {
  Serial.println();
  Serial.println("AUTOTERM Air 2D — stage 2 transparent bridge");
  Serial.println("Forwards bytes unchanged: panel <-> heater.");
  Serial.println("USB log shows short packet meaning (mode decode). No injected commands.");
  Serial.println("ECU keeps fuel pump, fan, glow plug, ignition, shutdown, temperature limits.");
  Serial.printf("UART: %lu %s\n",
                static_cast<unsigned long>(kBaudRate),
                serialFormatName(kSerialConfig));

  if (!pinsAreDistinct(kPanelRxPin, kPanelTxPin, kHeaterRxPin, kHeaterTxPin)) {
    Serial.println("ERROR: RX/TX pins overlap in Config.h. Fix them and reset.");
    return;
  }

  panel_.start(kPanelRxPin, kPanelTxPin, kBaudRate, kSerialConfig, kInvertUart, rxPullup_);
  heater_.start(kHeaterRxPin, kHeaterTxPin, kBaudRate, kSerialConfig, kInvertUart, rxPullup_);
  printPins();
}

void Sniffer::poll() {
  pollOne(panel_, heater_, panelAssembler_, panelBurst_, panelStats_, heaterStats_);
  pollOne(heater_, panel_, heaterAssembler_, heaterBurst_, heaterStats_, panelStats_);
}

void Sniffer::pollOne(UartEndpoint& source,
                      UartEndpoint& peer,
                      FrameAssembler& assembler,
                      BurstBuffer& burst,
                      PortStats& rxStats,
                      PortStats& txStats) {
  if (!source.isOpen()) {
    return;
  }

  const int waiting = source.available();
  if (waiting > rxStats.peakAvailable) {
    rxStats.peakAvailable = waiting;
  }

  const uint32_t now = micros();
  if (logMode_ == LogMode::Hex && burst.active && static_cast<uint32_t>(now - burst.lastUs) >= gapUs_) {
    flushBurst(source, burst, rxStats, false);
  }

  while (source.available() > 0) {
    const int raw = source.readByte();
    if (raw < 0) {
      break;
    }
    const uint8_t byte = static_cast<uint8_t>(raw);
    const uint32_t timestamp = micros();
    const bool hadByte = rxStats.hasByte;
    const uint32_t delta = timestamp - rxStats.lastByteUs;

    // Forward immediately — do not wait for a full packet.
    if (peer.writeByte(byte) == 1) {
      txStats.bytesTx++;
    }

    rxStats.bytesRx++;
    rxStats.lastByteUs = timestamp;
    rxStats.hasByte = true;

    if (logMode_ == LogMode::Bytes) {
      TrafficLogger::logByte(source.direction(), timestamp, delta, byte, hadByte);
      continue;
    }

    if (logMode_ == LogMode::Decode) {
      uint8_t frame[5 + 255 + 2];
      size_t frameLen = 0;
      uint32_t firstUs = 0;
      if (assembler.feed(byte, frame, sizeof(frame), frameLen, timestamp, firstUs)) {
        char text[96];
        if (PacketDecoder::describe(frame, frameLen, text, sizeof(text))) {
          TrafficLogger::logDecoded(source.name(), firstUs, text);
          rxStats.frames++;
        }
      }
      continue;
    }

    // Hex mode — gap-based burst dump.
    if (!burst.active) {
      burst.active = true;
      burst.len = 0;
      burst.firstUs = timestamp;
      rxStats.frames++;
    }

    if (burst.len >= kBurstBytes) {
      flushBurst(source, burst, rxStats, true);
      burst.active = true;
      burst.len = 0;
      burst.firstUs = timestamp;
    }

    burst.data[burst.len++] = byte;
    burst.lastUs = timestamp;
  }
}

void Sniffer::flushBurst(UartEndpoint& port, BurstBuffer& burst, PortStats& stats, bool split) {
  if (!burst.active || burst.len == 0) {
    clearBurst(burst);
    return;
  }
  if (split) {
    stats.splitFlushes++;
  }
  TrafficLogger::logBurst(port.direction(), burst.firstUs, burst.data, burst.len, split);
  clearBurst(burst);
}

void Sniffer::flushOpenBursts() {
  flushBurst(panel_, panelBurst_, panelStats_, false);
  flushBurst(heater_, heaterBurst_, heaterStats_, false);
}

void Sniffer::resetAssemblers() {
  panelAssembler_.reset();
  heaterAssembler_.reset();
}

bool Sniffer::setBaud(uint32_t baud) {
  if (baud < kBaudMin || baud > kBaudMax) {
    Serial.printf("baud must be %lu..%lu\n",
                  static_cast<unsigned long>(kBaudMin),
                  static_cast<unsigned long>(kBaudMax));
    return false;
  }
  flushOpenBursts();
  resetAssemblers();
  const bool panelOk = panel_.setBaud(baud);
  const bool heaterOk = heater_.setBaud(baud);
  Serial.printf("both UARTs set to %lu\n", static_cast<unsigned long>(baud));
  return panelOk && heaterOk;
}

bool Sniffer::setBaud(PortId port, uint32_t baud) {
  if (baud < kBaudMin || baud > kBaudMax) {
    Serial.printf("baud must be %lu..%lu\n",
                  static_cast<unsigned long>(kBaudMin),
                  static_cast<unsigned long>(kBaudMax));
    return false;
  }
  flushOpenBursts();
  resetAssemblers();
  UartEndpoint& endpoint = (port == PortId::Panel) ? panel_ : heater_;
  const bool ok = endpoint.setBaud(baud);
  Serial.printf("%s set to %lu\n", endpoint.name(), static_cast<unsigned long>(baud));
  return ok;
}

bool Sniffer::setFormat(uint32_t serialConfig) {
  flushOpenBursts();
  resetAssemblers();
  const bool panelOk = panel_.setSerialConfig(serialConfig);
  const bool heaterOk = heater_.setSerialConfig(serialConfig);
  Serial.printf("both UARTs set to %s\n", serialFormatName(serialConfig));
  return panelOk && heaterOk;
}

bool Sniffer::setFormat(PortId port, uint32_t serialConfig) {
  flushOpenBursts();
  resetAssemblers();
  UartEndpoint& endpoint = (port == PortId::Panel) ? panel_ : heater_;
  const bool ok = endpoint.setSerialConfig(serialConfig);
  Serial.printf("%s set to %s\n", endpoint.name(), serialFormatName(serialConfig));
  return ok;
}

bool Sniffer::setInvert(bool invert) {
  flushOpenBursts();
  resetAssemblers();
  const bool panelOk = panel_.setInvert(invert);
  const bool heaterOk = heater_.setInvert(invert);
  Serial.printf("invert %s\n", invert ? "on" : "off");
  return panelOk && heaterOk;
}

bool Sniffer::setGapMs(uint16_t gapMs) {
  if (gapMs < kGapMsMin || gapMs > kGapMsMax) {
    Serial.printf("gap must be %u..%u ms\n", kGapMsMin, kGapMsMax);
    return false;
  }
  flushOpenBursts();
  gapMs_ = gapMs;
  gapUs_ = static_cast<uint32_t>(gapMs) * 1000UL;
  Serial.printf("frame gap set to %u ms (hex log grouping only)\n", gapMs);
  return true;
}

bool Sniffer::setRxPin(PortId port, int8_t pin) {
  UartEndpoint& endpoint = (port == PortId::Panel) ? panel_ : heater_;
  UartEndpoint& other = (port == PortId::Panel) ? heater_ : panel_;
  if (pin == other.rxPin() || pin == other.txPin() || pin == endpoint.txPin()) {
    Serial.println("that GPIO is already used by the other signal");
    return false;
  }
  flushOpenBursts();
  resetAssemblers();
  const bool ok = endpoint.setRxPin(pin);
  if (ok) {
    Serial.printf("%s RX is now GPIO %d. TX stays GPIO %d.\n", endpoint.name(), pin, endpoint.txPin());
  }
  return ok;
}

void Sniffer::setLogMode(LogMode mode) {
  flushOpenBursts();
  resetAssemblers();
  logMode_ = mode;
  Serial.printf("log mode: %s\n", logModeName(mode));
}

void Sniffer::resetStats() {
  clearStats(panelStats_);
  clearStats(heaterStats_);
  Serial.println("counters cleared");
}

bool Sniffer::injectToHeater(const uint8_t* frame, size_t len) {
  if (frame == nullptr || len == 0 || !heater_.isOpen()) {
    return false;
  }
  const size_t written = heater_.write(frame, len);
  heaterStats_.bytesTx += static_cast<uint32_t>(written);

  char text[96];
  if (PacketDecoder::describe(frame, len, text, sizeof(text))) {
    TrafficLogger::logDecoded("INJECT", micros(), text);
  } else {
    Serial.println("INJECT: sent (undecoded)");
  }
  return written == len;
}

void Sniffer::printStatus() const {
  Serial.println("stage 2 bridge — bytes forwarded unchanged, no injection");
  Serial.printf("log: %s   gap: %u ms\n", logModeName(logMode_), gapMs_);
  printPortStatus(panel_, panelStats_);
  printPortStatus(heater_, heaterStats_);
  Serial.println("runtime changes are lost on reset — edit Config.h to keep them");
}

void Sniffer::printPins() const {
  Serial.println("pins (both RX and TX are active):");
  if (panel_.isOpen()) {
    Serial.printf("  PANEL  UART1  RX %d  TX %d\n", panel_.rxPin(), panel_.txPin());
  } else {
    Serial.println("  PANEL  UART1  not started");
  }
  if (heater_.isOpen()) {
    Serial.printf("  HEATER UART2  RX %d  TX %d\n", heater_.rxPin(), heater_.txPin());
  } else {
    Serial.println("  HEATER UART2  not started");
  }
}
