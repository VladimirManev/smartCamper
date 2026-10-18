/**
 * AUTOTERM Air 2D — stage 2 bridge + BOOT button injection test.
 *
 * UART1 <-> Comfort Control
 * UART2 <-> Air 2D ECU
 * UART0 = USB log / commands
 *
 * BOOT clicks: ventilation level 1..5, then STOP. ECU keeps all safety functions.
 */

#include <Arduino.h>

#include "BootInjectButton.h"
#include "Sniffer.h"
#include "UsbConsole.h"

Sniffer sniffer(Serial1, Serial2);
UsbConsole console(sniffer);
BootInjectButton bootButton(sniffer);

void setup() {
  Serial.begin(kUsbBaud);
  delay(50);
  sniffer.begin();
  console.begin();
  bootButton.begin();
}

void loop() {
  sniffer.poll();
  console.poll();
  bootButton.poll();
}
