/**
 * Fiat Ducato B-CAN — all doors decode test.
 * ID 0x06214000 byte1:
 *   driver 0x04, passenger 0x08, sliding 0x30, rear 0x40
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

static const uint8_t MASK_DRIVER = 0x04;
static const uint8_t MASK_PASSENGER = 0x08;
static const uint8_t MASK_SLIDING = 0x30;
static const uint8_t MASK_REAR = 0x40;

struct DoorState {
  bool driver;
  bool passenger;
  bool sliding;
  bool rear;
};

static bool haveState = false;
static DoorState state = {};

static void printDoor(const char *name, bool open) {
  Serial.printf("%s %s\n", name, open ? "OPEN" : "CLOSED");
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("B-CAN all doors test  50 kbit/s  OBD 1+9");
  Serial.println("ID E 06214000 byte1 masks: D04 P08 S30 R40");

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

  const uint8_t b = msg.data[DOOR_BYTE];
  DoorState next = {
      .driver = (b & MASK_DRIVER) != 0,
      .passenger = (b & MASK_PASSENGER) != 0,
      .sliding = (b & MASK_SLIDING) != 0,
      .rear = (b & MASK_REAR) != 0,
  };

  if (!haveState) {
    haveState = true;
    state = next;
    printDoor("DRIVER", state.driver);
    printDoor("PASSENGER", state.passenger);
    printDoor("SLIDING", state.sliding);
    printDoor("REAR", state.rear);
    return;
  }

  if (next.driver != state.driver) {
    printDoor("DRIVER", next.driver);
  }
  if (next.passenger != state.passenger) {
    printDoor("PASSENGER", next.passenger);
  }
  if (next.sliding != state.sliding) {
    printDoor("SLIDING", next.sliding);
  }
  if (next.rear != state.rear) {
    printDoor("REAR", next.rear);
  }
  state = next;
}
