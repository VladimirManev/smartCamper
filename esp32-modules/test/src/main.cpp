/**
 * Fiat Ducato B-CAN sniffer (listen-only) — ESP32 + WCMCU-230.
 *
 * Prints only when an ID changes after being stable (filters counters).
 *
 * Known doors — E 06214000 byte1:
 *   driver 0x04, passenger 0x08, sliding 0x30, rear 0x40
 *
 * Wiring:
 *   WCMCU-230 3V3  -> ESP32 3.3V
 *   WCMCU-230 GND  -> ESP32 GND + vehicle GND (OBD pin 4 or 5)
 *   WCMCU-230 CTX  -> ESP32 GPIO 17 (TX)
 *   WCMCU-230 CRX  -> ESP32 GPIO 16 (RX)
 *   WCMCU-230 CANH -> OBD pin 1
 *   WCMCU-230 CANL -> OBD pin 9
 *
 * Upload:  cd esp32-modules/test && pio run -t upload && pio device monitor
 * Wait for "Ready", then do one action at a time.
 */

#include <Arduino.h>
#include "driver/twai.h"
#include <string.h>

static const gpio_num_t CAN_TX_PIN = GPIO_NUM_17;
static const gpio_num_t CAN_RX_PIN = GPIO_NUM_16;

#define CAN_TIMING TWAI_TIMING_CONFIG_50KBITS()

static const unsigned WARMUP_MS = 3000;
static const unsigned STABLE_MS = 500;
static const int CACHE_SIZE = 128;

struct CachedFrame {
  uint32_t id;
  uint8_t extd;
  uint8_t dlc;
  uint8_t data[8];
  unsigned long lastChangeMs;
  bool used;
};

static CachedFrame cache[CACHE_SIZE];
static bool ready = false;
static unsigned long bootMs = 0;

static void printFrame(const twai_message_t &msg) {
  if (msg.extd) {
    Serial.printf("E %08X [%u] ", msg.identifier, msg.data_length_code);
  } else {
    Serial.printf("  %03X [%u] ", msg.identifier, msg.data_length_code);
  }
  for (int i = 0; i < msg.data_length_code; i++) {
    Serial.printf("%02X ", msg.data[i]);
  }
  Serial.println();
}

static CachedFrame *findOrAlloc(uint32_t id, uint8_t extd) {
  CachedFrame *freeSlot = nullptr;
  for (int i = 0; i < CACHE_SIZE; i++) {
    if (cache[i].used && cache[i].id == id && cache[i].extd == extd) {
      return &cache[i];
    }
    if (!cache[i].used && freeSlot == nullptr) {
      freeSlot = &cache[i];
    }
  }
  return freeSlot;
}

static void handleFrame(const twai_message_t &msg) {
  CachedFrame *slot = findOrAlloc(msg.identifier, msg.extd);
  if (slot == nullptr) {
    return;
  }

  const uint8_t dlc = msg.data_length_code;
  const unsigned long now = millis();

  if (!slot->used) {
    slot->used = true;
    slot->id = msg.identifier;
    slot->extd = msg.extd;
    slot->dlc = dlc;
    memcpy(slot->data, msg.data, dlc);
    slot->lastChangeMs = now;
    return;
  }

  bool changed =
      (slot->dlc != dlc) || (memcmp(slot->data, msg.data, dlc) != 0);
  if (!changed) {
    return;
  }

  const bool wasStable = (now - slot->lastChangeMs) >= STABLE_MS;
  slot->dlc = dlc;
  memcpy(slot->data, msg.data, dlc);
  slot->lastChangeMs = now;

  if (ready && wasStable) {
    printFrame(msg);
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("B-CAN sniffer listen-only (stable-change)");
  Serial.printf("TX=GPIO%d RX=GPIO%d  50 kbit/s  OBD 1+9  stable>=%ums\n",
                (int)CAN_TX_PIN, (int)CAN_RX_PIN, STABLE_MS);
  Serial.println("Doors known on E 06214000 byte1 — ignore those for new hunts");

  memset(cache, 0, sizeof(cache));
  bootMs = millis();

  twai_general_config_t g_config =
      TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_LISTEN_ONLY);
  twai_timing_config_t t_config = CAN_TIMING;
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
    Serial.println("TWAI install failed");
    return;
  }
  if (twai_start() != ESP_OK) {
    Serial.println("TWAI start failed");
    return;
  }
  Serial.printf("Warmup %u ms...\n", WARMUP_MS);
}

void loop() {
  if (!ready && (millis() - bootMs >= WARMUP_MS)) {
    ready = true;
    Serial.println("Ready — one action at a time");
  }

  twai_message_t msg;
  if (twai_receive(&msg, pdMS_TO_TICKS(50)) == ESP_OK) {
    if (!msg.rtr) {
      handleFrame(msg);
    }
  }
}
