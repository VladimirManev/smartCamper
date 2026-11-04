/**
 * Bench test: two DS18B20 sensors (same part as gray-water temp) and one
 * active-low contact input.
 *
 * Contact (GPIO 32): internal pull-up. Wire the switch between the pin and GND.
 * LOW means the pin is tied to ground.
 *
 * Each DS18B20 needs its own data pin. VCC may be 5V (same as the other
 * sensors in this project). The 4.7k pull-up must go from data to 3.3V, not
 * to 5V: the ESP32 pin is not 5V tolerant, and the idle bus level is set by
 * that resistor. The internal pull-up is too weak for OneWire. 12-bit
 * conversion takes about 750 ms; both sensors convert at the same time so a
 * 1 s print interval still fits.
 */

#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>

static const int PIN_CONTACT = 32;
static const int PIN_TEMP_1 = 25;
static const int PIN_TEMP_2 = 26;

static const unsigned long PRINT_INTERVAL_MS = 1000;
static const unsigned long CONVERSION_MS = 800;

OneWire oneWire1(PIN_TEMP_1);
OneWire oneWire2(PIN_TEMP_2);
DallasTemperature sensor1(&oneWire1);
DallasTemperature sensor2(&oneWire2);

static void printTemp(float celsius) {
  if (celsius == DEVICE_DISCONNECTED_C) {
    Serial.print("ERR");
    return;
  }
  Serial.print(celsius, 1);
  Serial.print(" C");
}

void setup() {
  Serial.begin(115200);
  delay(300);

  pinMode(PIN_CONTACT, INPUT_PULLUP);

  sensor1.begin();
  sensor2.begin();
  sensor1.setResolution(12);
  sensor2.setResolution(12);
  sensor1.setWaitForConversion(false);
  sensor2.setWaitForConversion(false);

  Serial.println("Test: DS18B20 x2 + pull-up input");
  Serial.println("GPIO 32 contact (LOW = GND), GPIO 25 temp1, GPIO 26 temp2");
  Serial.println("T1 devices: " + String(sensor1.getDeviceCount()));
  Serial.println("T2 devices: " + String(sensor2.getDeviceCount()));
}

void loop() {
  unsigned long started = millis();

  sensor1.requestTemperatures();
  sensor2.requestTemperatures();
  delay(CONVERSION_MS);

  float t1 = sensor1.getTempCByIndex(0);
  float t2 = sensor2.getTempCByIndex(0);
  bool tiedToGround = digitalRead(PIN_CONTACT) == LOW;

  Serial.print("T1: ");
  printTemp(t1);
  Serial.print(" | T2: ");
  printTemp(t2);
  Serial.print(" | pin32: ");
  Serial.println(tiedToGround ? "GND" : "OPEN");

  unsigned long elapsed = millis() - started;
  if (elapsed < PRINT_INTERVAL_MS) {
    delay(PRINT_INTERVAL_MS - elapsed);
  }
}
