#include "BootInjectButton.h"

#include "AutotermCommands.h"
#include "Config.h"

BootInjectButton::BootInjectButton(Sniffer& sniffer)
    : sniffer_(sniffer),
      ventLevel_(0),
      lastRaw_(HIGH),
      stablePressed_(false),
      lastChangeMs_(0),
      pressStartMs_(0) {}

void BootInjectButton::begin() {
  pinMode(kBootButtonPin, INPUT_PULLUP);
  lastRaw_ = digitalRead(kBootButtonPin);
  stablePressed_ = (lastRaw_ == LOW);
  lastChangeMs_ = millis();
  Serial.printf("BOOT button GPIO %d: click 1-5 = VENT level 1-5, click 6 = STOP\n", kBootButtonPin);
}

void BootInjectButton::poll() {
  const int raw = digitalRead(kBootButtonPin);
  const uint32_t now = millis();

  if (raw != lastRaw_) {
    lastRaw_ = raw;
    lastChangeMs_ = now;
    return;
  }

  if (static_cast<uint32_t>(now - lastChangeMs_) < kBootDebounceMs) {
    return;
  }

  const bool pressed = (raw == LOW);
  if (pressed == stablePressed_) {
    return;
  }

  if (pressed) {
    pressStartMs_ = now;
    stablePressed_ = true;
    return;
  }

  stablePressed_ = false;
  const uint32_t held = now - pressStartMs_;
  if (held < 20 || held > 2000) {
    return;
  }
  onClick();
}

bool BootInjectButton::sendTwice(const uint8_t* frame, size_t len) {
  // Panel often sends vent/start twice in short succession.
  if (!sniffer_.injectToHeater(frame, len)) {
    return false;
  }
  delay(30);
  return sniffer_.injectToHeater(frame, len);
}

void BootInjectButton::onClick() {
  uint8_t frame[16];
  size_t len = 0;

  if (ventLevel_ >= 5) {
    len = AutotermCommands::buildStop(frame, sizeof(frame));
    ventLevel_ = 0;
    if (len == 0 || !sendTwice(frame, len)) {
      Serial.println("INJECT: STOP failed");
    }
    return;
  }

  ventLevel_++;
  len = AutotermCommands::buildVent(ventLevel_, frame, sizeof(frame));
  if (len == 0 || !sendTwice(frame, len)) {
    Serial.printf("INJECT: VENT %u failed\n", ventLevel_);
    ventLevel_--;
  }
}
