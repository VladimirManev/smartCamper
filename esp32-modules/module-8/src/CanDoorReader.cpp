#include "CanDoorReader.h"
#include "Config.h"
#include "driver/twai.h"

CanDoorReader::CanDoorReader()
  : ready(false),
    haveState(false),
    changed(false),
    state({false, false, false, false}) {}

void CanDoorReader::begin() {
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
      (gpio_num_t)CAN_TX_PIN, (gpio_num_t)CAN_RX_PIN, TWAI_MODE_LISTEN_ONLY);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_50KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
    if (DEBUG_SERIAL) {
      Serial.println("CanDoorReader: TWAI install failed");
    }
    return;
  }
  if (twai_start() != ESP_OK) {
    if (DEBUG_SERIAL) {
      Serial.println("CanDoorReader: TWAI start failed");
    }
    twai_driver_uninstall();
    return;
  }

  ready = true;
  if (DEBUG_SERIAL) {
    Serial.println("CanDoorReader: B-CAN listen-only 50 kbit/s (GPIO16/17)");
  }
}

void CanDoorReader::handleFrame(uint32_t id, const uint8_t *data, uint8_t dlc) {
  if (id != CAN_DOOR_ID || dlc <= CAN_DOOR_BYTE) {
    return;
  }

  const uint8_t b = data[CAN_DOOR_BYTE];
  CanDoorState next = {
      .driver = (b & CAN_MASK_DRIVER) != 0,
      .passenger = (b & CAN_MASK_PASSENGER) != 0,
      .sliding = (b & CAN_MASK_SLIDING) != 0,
      .rear = (b & CAN_MASK_REAR) != 0,
  };

  if (!haveState) {
    haveState = true;
    state = next;
    changed = true;
    if (DEBUG_SERIAL) {
      Serial.printf("CAN doors: D=%d P=%d S=%d R=%d\n", state.driver,
                    state.passenger, state.sliding, state.rear);
    }
    return;
  }

  if (next.driver != state.driver || next.passenger != state.passenger ||
      next.sliding != state.sliding || next.rear != state.rear) {
    if (DEBUG_SERIAL) {
      if (next.driver != state.driver) {
        Serial.printf("DRIVER_DOOR %s\n", next.driver ? "OPEN" : "CLOSED");
      }
      if (next.passenger != state.passenger) {
        Serial.printf("PASSENGER_DOOR %s\n",
                      next.passenger ? "OPEN" : "CLOSED");
      }
      if (next.sliding != state.sliding) {
        Serial.printf("SLIDING_DOOR %s\n", next.sliding ? "OPEN" : "CLOSED");
      }
      if (next.rear != state.rear) {
        Serial.printf("REAR_DOOR %s\n", next.rear ? "OPEN" : "CLOSED");
      }
    }
    state = next;
    changed = true;
  }
}

void CanDoorReader::loop() {
  if (!ready) {
    return;
  }

  twai_message_t msg;
  // Drain available frames without blocking WiFi/MQTT loop long
  while (twai_receive(&msg, 0) == ESP_OK) {
    if (!msg.rtr && msg.extd) {
      handleFrame(msg.identifier, msg.data, msg.data_length_code);
    }
  }
}

bool CanDoorReader::consumeChanged() {
  if (!changed) {
    return false;
  }
  changed = false;
  return true;
}
