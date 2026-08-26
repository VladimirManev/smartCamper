/**
 * Fiat Ducato CAN sniffer (listen-only) — ESP32 + WCMCU-230 (SN65HVD230).
 *
 * Wiring:
 *   WCMCU-230 3V3  -> ESP32 3.3V
 *   WCMCU-230 GND  -> ESP32 GND + vehicle GND (OBD pin 4 or 5)
 *   WCMCU-230 CTX  -> ESP32 GPIO 17 (TX)
 *   WCMCU-230 CRX  -> ESP32 GPIO 16 (RX)
 *   WCMCU-230 CANH -> OBD pin 6
 *   WCMCU-230 CANL -> OBD pin 14
 *
 * Do NOT enable the 120 ohm terminator on WCMCU-230 when tapping the car bus.
 *
 * Upload:  cd esp32-modules/test && pio run -t upload && pio device monitor
 * Open/close a door and watch which ID/bytes change.
 */

#include <Arduino.h>
#include "driver/twai.h"

// Free on module-8 (used pins: 0,4,5,18,19,21,22,23,25,26,27,32,33)
static const gpio_num_t CAN_TX_PIN = GPIO_NUM_17;
static const gpio_num_t CAN_RX_PIN = GPIO_NUM_16;

// OBD diagnostic CAN is usually 500 kbit/s. Change if you get no frames:
//   TWAI_TIMING_CONFIG_125KBITS() / TWAI_TIMING_CONFIG_50KBITS()
#define CAN_TIMING TWAI_TIMING_CONFIG_500KBITS()

static void printFrame(const twai_message_t &msg) {
  Serial.printf("%03X [%u] ", msg.identifier, msg.data_length_code);
  for (int i = 0; i < msg.data_length_code; i++) {
    Serial.printf("%02X ", msg.data[i]);
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("CAN sniffer listen-only");
  Serial.printf("TX=GPIO%d RX=GPIO%d  500 kbit/s\n", (int)CAN_TX_PIN,
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
  Serial.println("Listening...");
}

void loop() {
  twai_message_t msg;
  if (twai_receive(&msg, pdMS_TO_TICKS(1000)) == ESP_OK) {
    if (!(msg.rtr)) {
      printFrame(msg);
    }
  }
}
