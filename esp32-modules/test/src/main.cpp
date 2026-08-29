/**
 * Ducato OBD 1+9 baud sweep (listen-only) — ESP32 + WCMCU-230.
 *
 * Cycles common low-speed rates; prints every frame + status.
 * Look for a rate where rx_total rises and bus_err stays low.
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
 * Key ON. Watch which baud produces frames.
 */

#include <Arduino.h>
#include "driver/twai.h"

static const gpio_num_t CAN_TX_PIN = GPIO_NUM_17;
static const gpio_num_t CAN_RX_PIN = GPIO_NUM_16;

static const unsigned DWELL_MS = 8000;   // time per baud
static const unsigned STATUS_MS = 2000;

// APB 80 MHz; bitrate = 80e6 / (brp * (1 + tseg_1 + tseg_2))
struct BaudStep {
  const char *label;
  twai_timing_config_t timing;
};

// tq=20 (1+15+4) except where noted
static const BaudStep BAUD_STEPS[] = {
    // ESP32 TWAI brp max 128 → ~25 kbit/s minimum at 80 MHz APB
    {"25 kbit/s",
     {.brp = 128, .tseg_1 = 16, .tseg_2 = 8, .sjw = 3, .triple_sampling = false}},
    {"33.3 kbit/s",
     {.brp = 120, .tseg_1 = 15, .tseg_2 = 4, .sjw = 3, .triple_sampling = false}},
    {"47.6 kbit/s",
     {.brp = 84, .tseg_1 = 15, .tseg_2 = 4, .sjw = 3, .triple_sampling = false}},
    {"50 kbit/s",
     {.brp = 80, .tseg_1 = 15, .tseg_2 = 4, .sjw = 3, .triple_sampling = false}},
    {"62.5 kbit/s",
     {.brp = 64, .tseg_1 = 15, .tseg_2 = 4, .sjw = 3, .triple_sampling = false}},
    {"83.3 kbit/s",
     {.brp = 48, .tseg_1 = 15, .tseg_2 = 4, .sjw = 3, .triple_sampling = false}},
    {"100 kbit/s",
     {.brp = 40, .tseg_1 = 15, .tseg_2 = 4, .sjw = 3, .triple_sampling = false}},
};

static const int NUM_BAUDS = sizeof(BAUD_STEPS) / sizeof(BAUD_STEPS[0]);

static int baudIndex = 0;
static bool driverUp = false;
static uint32_t rxCount = 0;
static unsigned long stepStartMs = 0;
static unsigned long lastStatusMs = 0;

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

static void printStatus() {
  twai_status_info_t s;
  if (twai_get_status_info(&s) != ESP_OK) {
    Serial.println("TWAI status read failed");
    return;
  }
  Serial.printf(
      "  STAT rx_total=%lu state=%u rx_err=%u bus_err=%lu rx_miss=%lu\n",
      (unsigned long)rxCount, (unsigned)s.state, (unsigned)s.rx_error_counter,
      (unsigned long)s.bus_error_count, (unsigned long)s.rx_missed_count);
}

static void stopTwai() {
  if (!driverUp) {
    return;
  }
  twai_stop();
  twai_driver_uninstall();
  driverUp = false;
}

static bool startTwai(const twai_timing_config_t &timing) {
  stopTwai();

  twai_general_config_t g_config =
      TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_LISTEN_ONLY);
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
  twai_timing_config_t t_config = timing;

  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
    Serial.println("TWAI install failed");
    return false;
  }
  if (twai_start() != ESP_OK) {
    Serial.println("TWAI start failed");
    twai_driver_uninstall();
    return false;
  }
  driverUp = true;
  return true;
}

static void beginBaudStep(int index) {
  baudIndex = index;
  rxCount = 0;
  stepStartMs = millis();
  lastStatusMs = stepStartMs;

  Serial.println();
  Serial.printf("=== BAUD %s (%d/%d) for %u ms ===\n", BAUD_STEPS[index].label,
                index + 1, NUM_BAUDS, DWELL_MS);

  if (!startTwai(BAUD_STEPS[index].timing)) {
    Serial.println("Skip — driver failed");
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("CAN baud sweep listen-only  OBD 1+9");
  Serial.printf("TX=GPIO%d RX=GPIO%d  dwell=%ums\n", (int)CAN_TX_PIN,
                (int)CAN_RX_PIN, DWELL_MS);
  Serial.println("Key ON. Watch for rising rx_total.");

  beginBaudStep(0);
}

void loop() {
  if (driverUp) {
    twai_message_t msg;
    while (twai_receive(&msg, 0) == ESP_OK) {
      if (!msg.rtr) {
        rxCount++;
        printFrame(msg);
      }
    }
  }

  const unsigned long now = millis();
  if (now - lastStatusMs >= STATUS_MS) {
    lastStatusMs = now;
    printStatus();
  }

  if (now - stepStartMs >= DWELL_MS) {
    beginBaudStep((baudIndex + 1) % NUM_BAUDS);
  }
}
