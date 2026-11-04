# esp32-modules/test

Bench sketch: two DS18B20 temperature sensors and one active-low contact input. Prints one line per second on the serial monitor at 115200 baud.

## Pins

| Signal | GPIO | Notes |
|--------|------|--------|
| Contact | 32 | `INPUT_PULLUP`. Switch between this pin and GND. `GND` = closed to ground, `OPEN` = released. |
| Temp 1 | 25 | DS18B20 data, same sensor as gray-water temperature on module-1 |
| Temp 2 | 26 | DS18B20 data |

Each DS18B20: VCC to 3.3V, GND to GND, data to the GPIO, and a 4.7k resistor from data to 3.3V. The ESP32 internal pull-up is not enough for OneWire. Do not feed a 5V data line into the ESP32.

12-bit conversion takes about 750 ms. Both sensors start together, so the sketch can still print once per second.

`ERR` means that sensor did not answer on its pin.
