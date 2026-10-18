#include "UartEndpoint.h"

#include "Config.h"
#include "Esp32Pins.h"

#include <driver/gpio.h>

UartEndpoint::UartEndpoint(HardwareSerial& serial, const char* name)
    : serial_(serial),
      name_(name),
      rxPin_(-1),
      txPin_(-1),
      baud_(0),
      serialConfig_(SERIAL_8N1),
      invert_(false),
      rxPullup_(false),
      open_(false) {}

bool UartEndpoint::start(int8_t rxPin, int8_t txPin, uint32_t baud, uint32_t serialConfig, bool invert,
                         bool rxPullup) {
  const PinCheck rx = checkRxPin(rxPin);
  if (!rx.ok) {
    Serial.printf("%s RX GPIO %d rejected: %s\n", name_, rxPin, rx.message);
    open_ = false;
    return false;
  }

  const PinCheck tx = checkReservedTxPin(txPin);
  if (!tx.ok) {
    Serial.printf("%s TX GPIO %d rejected: %s\n", name_, txPin, tx.message);
    open_ = false;
    return false;
  }

  if (rxPin == txPin) {
    Serial.printf("%s RX and TX cannot be the same GPIO\n", name_);
    open_ = false;
    return false;
  }

  rxPin_ = rxPin;
  txPin_ = txPin;
  baud_ = baud;
  serialConfig_ = serialConfig;
  invert_ = invert;
  rxPullup_ = rxPullup;

  if (rx.warning) {
    Serial.printf("%s RX GPIO %d: %s\n", name_, rxPin, rx.message);
  }
  if (tx.warning) {
    Serial.printf("%s TX GPIO %d: %s\n", name_, txPin, tx.message);
  }

  return restart();
}

bool UartEndpoint::restart() {
  if (rxPin_ < 0 || txPin_ < 0) {
    Serial.printf("%s RX/TX are not configured\n", name_);
    open_ = false;
    return false;
  }

  if (open_) {
    serial_.end();
    open_ = false;
  }

  serial_.setRxBufferSize(AUTOTERM_RX_BUFFER_BYTES);
  serial_.setTxBufferSize(AUTOTERM_TX_BUFFER_BYTES);
  serial_.begin(baud_, serialConfig_, rxPin_, txPin_, invert_);
  serial_.setTimeout(1);
  applyRxPullup();
  open_ = true;
  return true;
}

int UartEndpoint::available() {
  if (!open_) {
    return 0;
  }
  return serial_.available();
}

int UartEndpoint::readByte() {
  if (!open_) {
    return -1;
  }
  return serial_.read();
}

size_t UartEndpoint::writeByte(uint8_t value) {
  if (!open_) {
    return 0;
  }
#if !AUTOTERM_BRIDGE_TX
#error "Autoterm bridge must be built with AUTOTERM_BRIDGE_TX=1"
#endif
  return serial_.write(value);
}

size_t UartEndpoint::write(const uint8_t* data, size_t len) {
  if (!open_ || data == nullptr || len == 0) {
    return 0;
  }
#if !AUTOTERM_BRIDGE_TX
#error "Autoterm bridge must be built with AUTOTERM_BRIDGE_TX=1"
#endif
  return serial_.write(data, len);
}

void UartEndpoint::applyRxPullup() const {
  if (!rxPullup_ || rxPin_ < 0) {
    return;
  }
  gpio_pullup_en(static_cast<gpio_num_t>(rxPin_));
  gpio_pulldown_dis(static_cast<gpio_num_t>(rxPin_));
}
