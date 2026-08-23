/**
 * LilyGO T-Zigbee bring-up test (SmartCamper fan RF project).
 *
 * Powers TLSR8258 and checks USB serial + HCI link to the Telink chip.
 * Based on LilyGO factory_test (WiFi tests removed).
 *
 * Hardware: DIP 1+2=ON, 3-5=OFF. CH340 @ 3.3V. Unplug LilyGO USB-C from Mac during upload.
 */

#include <Arduino.h>
#include <zbhci.h>

static const uint8_t TLSR8258_POWER_PIN = 0;
static const uint8_t STATUS_LED_PIN = 3;

static QueueHandle_t hciMsgQueue;

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("SmartCamper - LilyGO T-Zigbee bring-up test");
  Serial.println("Board: ESP32-C3 + TLSR8258");
  Serial.println("DIP switches: 1+2=ON, 3-5=OFF (ESP32-C3 UART mode)");
  Serial.println();

  pinMode(TLSR8258_POWER_PIN, OUTPUT);
  digitalWrite(TLSR8258_POWER_PIN, HIGH);
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);

  Serial.println("TLSR8258 power: ON (GPIO0 HIGH)");

  hciMsgQueue = xQueueCreate(10, sizeof(ts_HciMsg));
  zbhci_Init(hciMsgQueue);
  delay(200);
  zbhci_NetworkStateReq();

  Serial.println("zbhci initialized");
  Serial.println("Waiting for TLSR8258 HCI messages...");
  Serial.println();
}

void loop() {
  ts_HciMsg msg;
  if (xQueueReceive(hciMsgQueue, &msg, pdMS_TO_TICKS(100)) == pdTRUE) {
    Serial.printf("HCI msg: 0x%04X\n", msg.u16MsgType);
    digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
  }

  static uint32_t lastHeartbeat = 0;
  uint32_t now = millis();
  if (now - lastHeartbeat >= 5000) {
    lastHeartbeat = now;
    Serial.printf("[%lu s] alive | TLSR8258 powered | heap %u\n",
                  now / 1000, ESP.getFreeHeap());
  }
}
