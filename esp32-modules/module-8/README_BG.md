# Module-8: Алармена система

Автономен ESP32 модул за SmartCamper: вътрешна зона (сирена + пушек), периметър (само зумер), MQTT статус/команди.

Работи без Pi (бутон + локална логика). WiFi/MQTT както при останалите модули.

## Хардуер (GPIO)

| Функция | Pin | Бележки |
|---------|-----|---------|
| BOOT бутон | 0 | Вграден |
| Spare контакт | 4 | `INPUT_PULLUP`, LOW = отворен |
| Interior PIR | 5 | HIGH = движение |
| Perimeter front/rear/… | 18,19,21,22,23,25 | 6 бр. |
| Зумер | 26 | |
| Сирена (реле) | 27 | |
| Пушек (реле) | 32 | |
| LED зона 1 | 33 | Мига при охрана |
| CAN TX (WCMCU CTX) | 17 | Ducato B-CAN listen-only |
| CAN RX (WCMCU CRX) | 16 | OBD 1+9, 50 kbit/s |

Пиновете са в `src/Config.h`.

Вратите идват от CAN (`0x06214000`) → MQTT `inputs.doors.*`. Докато шината спи, се държи последното известно състояние.

## Бутон

| Последователност | Действие |
|------------------|----------|
| 1 кратко + hold ≥3 s | Arm зона 1 нормално, или **disarm** зона 1 (normal или cat) |
| 2 кратки + hold ≥3 s | Toggle зона 2 |
| 3 кратки + hold ≥3 s | Само arm зона 1 cat mode |
| Друго | Дълъг error beep |

Докато зона 1 е armed/arming, cat последователността дава error — disarm винаги с **1 кратко + hold**.

## MQTT (накратко)

- Status: `smartcamper/sensors/module-8/status`
- Heartbeat: `smartcamper/heartbeat/module-8`
- Arm Z1: `.../zone/1/on` (+ optional `ignoreInteriorPir`)
- Disarm Z1: `.../zone/1/off` + `{"pin":"1234"}`
- Z2 on/off: без PIN
- PIN по подразбиране: `1234` в `Config.h`

Пълни детайли: [README.md](README.md)
