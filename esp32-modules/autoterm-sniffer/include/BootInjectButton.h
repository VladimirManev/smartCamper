// BOOT button (GPIO 0) injection test:
// click 1..5 -> ventilation level 1..5, click 6 -> STOP, then repeat.

#ifndef AUTOTERM_BOOT_INJECT_BUTTON_H
#define AUTOTERM_BOOT_INJECT_BUTTON_H

#include "Sniffer.h"

class BootInjectButton {
 public:
  explicit BootInjectButton(Sniffer& sniffer);

  void begin();
  void poll();

 private:
  void onClick();
  bool sendTwice(const uint8_t* frame, size_t len);

  Sniffer& sniffer_;
  uint8_t ventLevel_;  // 0 = off, 1..5 = ventilation level
  int lastRaw_;
  bool stablePressed_;
  uint32_t lastChangeMs_;
  uint32_t pressStartMs_;
};

#endif
