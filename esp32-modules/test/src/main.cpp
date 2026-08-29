/**
 * Fiat Ducato B-CAN raw sniffer (listen-only) — ESP32 + WCMCU-230.
 *
 * Diagnostic mode: print every frame + TWAI status every 2s.
 *
 * Wiring:
 *   WCMCU-230 3V3  -> ESP32 3.3V
 *   WCMCU-230 GND  -> ESP32 GND + vehicle GND (OBD pin 4 or 5)
 *   WCMCU-230 CTX  -> ESP32 GPIO 17 (TX)
 *   WCMCU-230 CRX  -> ESP32 GPIO 16 (RX)
 *   WCMCU-230 CANH -> OBD pin 1  (B-CAN)
 *   WCMCU-230 CANL -> OBD pin 9
 *
 * Upload:  cd esp32-modules/test && pio run -t upload && pio device monitor
 */

#include <Arduino.h>
#include "driver/twai.h"

static const gpio_num_t CAN_TX_PIN = GPIO_NUM_17;
static const gpio_num_t CAN_RX_PIN = GPIO_NUM_16;

#define CAN_TIMING TWAI_TIMING_CONFIG_50KBITS()

static const unsigned STATUS_MS = 2000;
static unsigned long lastStatusMs = 0;
static uint32_t rxCount = 0;

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
      "STAT state=%u rx_err=%u tx_err=%u rx_miss=%lu arb_lost=%lu "
      "bus_err=%lu rx_q=%u\n",
      (unsigned)s.state, (unsigned)s.rx_error_counter,
      (unsigned)s.tx_error_counter, (unsigned long)s.rx_missed_count,
      (unsigned long)s.arb_lost_count, (unsigned long)s.bus_error_count,
      (unsigned)s.msgs_to_rx);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("CAN sniffer listen-only B-CAN RAW");
  Serial.printf("TX=GPIO%d RX=GPIO%d  50 kbit/s  OBD 1+9\n", (int)CAN_TX_PIN,
                (int)CAN_RX_PIN);

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
  Serial.println("Listening (raw)...");
  lastStatusMs = millis();
}

void loop() {
  twai_message_t msg;
  if (twai_receive(&msg, pdMS_TO_TICKS(50)) == ESP_OK) {
    if (!msg.rtr) {
      rxCount++;
      printFrame(msg);
    }
  }

  if (millis() - lastStatusMs >= STATUS_MS) {
    lastStatusMs = millis();
    Serial.printf("rx_total=%lu  ", (unsigned long)rxCount);
    printStatus();
  }
}
