/**
 * Fiat Ducato 2015 B-CAN decode test — ESP32 + WCMCU-230.
 *
 * OBD pins 1+9, 50 kbit/s, listen-only.
 * TX=GPIO17, RX=GPIO16.
 *
 * Upload: cd esp32-modules/test && pio run -t upload && pio device monitor
 *
 * === Signal map (discovered) ===
 * E 06214000
 *   byte0 bit 0x20  handbrake ON
 *   byte1 bit 0x04  driver door open
 *   byte1 bit 0x08  passenger door open
 *   byte1 bit 0x30  sliding door open
 *   byte1 bit 0x40  rear door open
 * E 02214000
 *   byte1 bit 0x60  parking lights ON
 *   byte1 bit 0x10  high beam / flash
 *   byte2 bit 0x20  right turn
 *   byte2 bit 0x40  left turn
 *   byte2 bit 0x60  hazards
 * E 02294000  (pulses)
 *   byte5 bit 0x80  lock (both zones)
 *   byte5 bit 0x08  unlock front (cockpit)
 *   byte6 bit 0x80  unlock rear
 * E 04214001
 *   byte7 bit 0x04  reverse gear ON
 * E 0E094024
 *   byte1 0x1A radio ON; OFF = 0x1E or ID stops (timeout ~2s)
 */

#include <Arduino.h>
#include "driver/twai.h"

static const gpio_num_t CAN_TX_PIN = GPIO_NUM_17;
static const gpio_num_t CAN_RX_PIN = GPIO_NUM_16;

#define CAN_TIMING TWAI_TIMING_CONFIG_50KBITS()

static const uint32_t ID_DOORS = 0x06214000;
static const uint32_t ID_LIGHTS = 0x02214000;
static const uint32_t ID_LOCKS = 0x02294000;
static const uint32_t ID_REVERSE = 0x04214001;
static const uint32_t ID_RADIO = 0x0E094024;

struct DoorLightState {
  bool handbrake;
  bool driver;
  bool passenger;
  bool sliding;
  bool rear;
  bool parking;
  bool highBeam;
  bool rightTurn;
  bool leftTurn;
  bool hazards;
  bool reverse;
  bool radioOn;
  bool radioKnown;
};

static bool haveBody = false;
static bool haveLights = false;
static bool haveReverse = false;
static DoorLightState st = {};

static const unsigned RADIO_OFF_TIMEOUT_MS = 2000;
static unsigned long lastRadioMs = 0;

static void logChange(const char *name, bool on) {
  Serial.printf("%s %s\n", name, on ? "ON" : "OFF");
}

static void logDoor(const char *name, bool open) {
  Serial.printf("%s %s\n", name, open ? "OPEN" : "CLOSED");
}

static void logPulse(const char *name) {
  Serial.printf("%s\n", name);
}

static uint8_t prevLockB5 = 0;
static uint8_t prevLockB6 = 0;
static bool haveLocks = false;

static void handleDoors(const twai_message_t &msg) {
  if (msg.data_length_code < 2) {
    return;
  }
  const uint8_t b0 = msg.data[0];
  const uint8_t b1 = msg.data[1];

  DoorLightState n = st;
  n.handbrake = (b0 & 0x20) != 0;
  n.driver = (b1 & 0x04) != 0;
  n.passenger = (b1 & 0x08) != 0;
  n.sliding = (b1 & 0x30) != 0;
  n.rear = (b1 & 0x40) != 0;

  if (!haveBody) {
    haveBody = true;
    st.handbrake = n.handbrake;
    st.driver = n.driver;
    st.passenger = n.passenger;
    st.sliding = n.sliding;
    st.rear = n.rear;
    logChange("HANDBRAKE", st.handbrake);
    logDoor("DRIVER_DOOR", st.driver);
    logDoor("PASSENGER_DOOR", st.passenger);
    logDoor("SLIDING_DOOR", st.sliding);
    logDoor("REAR_DOOR", st.rear);
    return;
  }

  if (n.handbrake != st.handbrake) {
    logChange("HANDBRAKE", n.handbrake);
  }
  if (n.driver != st.driver) {
    logDoor("DRIVER_DOOR", n.driver);
  }
  if (n.passenger != st.passenger) {
    logDoor("PASSENGER_DOOR", n.passenger);
  }
  if (n.sliding != st.sliding) {
    logDoor("SLIDING_DOOR", n.sliding);
  }
  if (n.rear != st.rear) {
    logDoor("REAR_DOOR", n.rear);
  }
  st.handbrake = n.handbrake;
  st.driver = n.driver;
  st.passenger = n.passenger;
  st.sliding = n.sliding;
  st.rear = n.rear;
}

static void handleLights(const twai_message_t &msg) {
  if (msg.data_length_code < 3) {
    return;
  }
  const uint8_t b1 = msg.data[1];
  const uint8_t b2 = msg.data[2];

  const bool parking = (b1 & 0x60) != 0;
  const bool highBeam = (b1 & 0x10) != 0;
  const bool rightTurn = (b2 & 0x20) != 0;
  const bool leftTurn = (b2 & 0x40) != 0;
  const bool hazards = (b2 & 0x60) == 0x60;

  if (!haveLights) {
    haveLights = true;
    st.parking = parking;
    st.highBeam = highBeam;
    st.rightTurn = rightTurn;
    st.leftTurn = leftTurn;
    st.hazards = hazards;
    logChange("PARKING_LIGHTS", st.parking);
    logChange("HIGH_BEAM", st.highBeam);
    logChange("RIGHT_TURN", st.rightTurn);
    logChange("LEFT_TURN", st.leftTurn);
    logChange("HAZARDS", st.hazards);
    return;
  }

  if (parking != st.parking) {
    logChange("PARKING_LIGHTS", parking);
  }
  if (highBeam != st.highBeam) {
    logChange("HIGH_BEAM", highBeam);
  }
  if (hazards != st.hazards) {
    logChange("HAZARDS", hazards);
  } else {
    if (rightTurn != st.rightTurn) {
      logChange("RIGHT_TURN", rightTurn);
    }
    if (leftTurn != st.leftTurn) {
      logChange("LEFT_TURN", leftTurn);
    }
  }
  st.parking = parking;
  st.highBeam = highBeam;
  st.rightTurn = rightTurn;
  st.leftTurn = leftTurn;
  st.hazards = hazards;
}

static void handleLocks(const twai_message_t &msg) {
  if (msg.data_length_code < 7) {
    return;
  }
  const uint8_t b5 = msg.data[5];
  const uint8_t b6 = msg.data[6];

  if (!haveLocks) {
    haveLocks = true;
    prevLockB5 = b5;
    prevLockB6 = b6;
    return;
  }

  if ((b5 & 0x80) && !(prevLockB5 & 0x80)) {
    logPulse("LOCK_ALL");
  }
  if ((b5 & 0x08) && !(prevLockB5 & 0x08)) {
    logPulse("UNLOCK_FRONT");
  }
  if ((b6 & 0x80) && !(prevLockB6 & 0x80)) {
    logPulse("UNLOCK_REAR");
  }
  prevLockB5 = b5;
  prevLockB6 = b6;
}

static void handleReverse(const twai_message_t &msg) {
  if (msg.data_length_code < 8) {
    return;
  }
  const bool on = (msg.data[7] & 0x04) != 0;
  if (!haveReverse) {
    haveReverse = true;
    st.reverse = on;
    logChange("REVERSE", st.reverse);
    return;
  }
  if (on != st.reverse) {
    logChange("REVERSE", on);
    st.reverse = on;
  }
}

static void handleRadio(const twai_message_t &msg) {
  if (msg.data_length_code < 2) {
    return;
  }
  const uint8_t b1 = msg.data[1];
  // 0x1A = ON. 0x1E is a transient on power-up; treat only 0x1A as ON.
  if (b1 != 0x1A && b1 != 0x1E) {
    return;
  }

  lastRadioMs = millis();

  if (b1 == 0x1A) {
    if (!st.radioKnown || !st.radioOn) {
      st.radioKnown = true;
      st.radioOn = true;
      logChange("RADIO", true);
    }
    return;
  }

  // 0x1E: ignore alone (power-up glitch). Real OFF is timeout below.
}

static void pollRadioTimeout() {
  if (!st.radioKnown || !st.radioOn) {
    return;
  }
  if (millis() - lastRadioMs < RADIO_OFF_TIMEOUT_MS) {
    return;
  }
  st.radioOn = false;
  logChange("RADIO", false);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("B-CAN full decode test  50 kbit/s  OBD 1+9");
  Serial.println("See signal map in main.cpp header");

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
  if (twai_receive(&msg, pdMS_TO_TICKS(50)) == ESP_OK) {
    if (!msg.rtr && msg.extd) {
      switch (msg.identifier) {
        case ID_DOORS:
          handleDoors(msg);
          break;
        case ID_LIGHTS:
          handleLights(msg);
          break;
        case ID_LOCKS:
          handleLocks(msg);
          break;
        case ID_REVERSE:
          handleReverse(msg);
          break;
        case ID_RADIO:
          handleRadio(msg);
          break;
        default:
          break;
      }
    }
  }
  pollRadioTimeout();
}
