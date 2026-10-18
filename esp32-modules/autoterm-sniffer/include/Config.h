// Stage 2 — transparent serial bridge for AUTOTERM Air 2D.
//
// Confirmed on this Comfort Control + Air 2D pair:
//   baud 9600 8N1, frames AA 03 (panel) / AA 04 (heater), CRC16 Modbus.
//
// Later stages (not implemented here):
//   1. Command injection — only bytes captured and confirmed
//   2. Wi-Fi / MQTT / Home Assistant

#ifndef AUTOTERM_CONFIG_H
#define AUTOTERM_CONFIG_H

#include <Arduino.h>

#ifndef AUTOTERM_BRIDGE_TX
#define AUTOTERM_BRIDGE_TX 1
#endif

static constexpr uint32_t kUsbBaud = 115200;

static constexpr uint32_t kBaudRate = 9600;
static constexpr uint32_t kSerialConfig = SERIAL_8N1;
static constexpr bool kInvertUart = false;

// Silence that closes one log line. Display only — not a protocol delimiter.
static constexpr uint16_t kFrameGapMs = 20;

// Match module-4 Autoterm pins (DevKit left header, USB at bottom).
// PANEL UART1: RX 34, TX 27  → Comfort Control
// HEATER UART2: RX 32, TX 33 → Air 2D ECU
static constexpr int8_t kPanelRxPin = 34;
static constexpr int8_t kPanelTxPin = 27;
static constexpr int8_t kHeaterRxPin = 32;
static constexpr int8_t kHeaterTxPin = 33;

static constexpr bool kRxPullup = true;

// DevKit BOOT button — injection test only (active LOW).
static constexpr int8_t kBootButtonPin = 0;
static constexpr uint16_t kBootDebounceMs = 40;

static constexpr size_t kRxBufferBytes = 2048;
static constexpr size_t kTxBufferBytes = 512;
static constexpr size_t kBurstBytes = 256;

static constexpr uint32_t kBaudMin = 300;
static constexpr uint32_t kBaudMax = 2000000;
static constexpr uint16_t kGapMsMin = 1;
static constexpr uint16_t kGapMsMax = 500;

enum class PortId : uint8_t { Panel = 0, Heater = 1 };

enum class LogMode : uint8_t { Decode = 0, Hex = 1, Bytes = 2 };

#endif
