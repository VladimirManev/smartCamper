/**
 * Fiat Ducato B-CAN — driver door decode test.
 * ID 0x06214000 byte1 bit 0x04: set = open, clear = closed.
 *
 * Wiring: WCMCU-230, OBD 1+9, GPIO 17 TX / 16 RX, 50 kbit/s listen-only.
 * Upload: cd esp32-modules/test && pio run -t upload && pio device monitor
 */

#include <Arduino.h>
#include "driver/twai.h"

static const gpio_num_t CAN_TX_PIN = GPIO_NUM_17;
static const gpio_num_t CAN_RX_PIN = GPIO_NUM_16;

#define CAN_TIMING TWAI_TIMING_CONFIG_50KBITS()

static const uint32_t DOOR_ID = 0x06214000;
static const uint8_t DOOR_BYTE = 1;
static const uint8_t DOOR_BIT = 0x04;

static bool haveState = false;
static bool doorOpen = false;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("B-CAN driver door test  50 kbit/s  OBD 1+9");
  Serial.println("Watching E 06214000 byte1 bit0x04");

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
  if (twai_receive(&msg, pdMS_TO_TICKS(100)) != ESP_OK) {
    return;
  }
  if (msg.rtr || !msg.extd || msg.identifier != DOOR_ID) {
    return;
  }
  if (msg.data_length_code <= DOOR_BYTE) {
    return;
  }

  const bool open = (msg.data[DOOR_BYTE] & DOOR_BIT) != 0;
  if (haveState && open == doorOpen) {
    return;
  }

  haveState = true;
  doorOpen = open;
  if (open) {
    Serial.println("DRIVER DOOR OPEN");
  } else {
    Serial.println("DRIVER DOOR CLOSED");
  }
}
