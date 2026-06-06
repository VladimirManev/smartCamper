# Module-6: Victron BLE енергиен монитор

Отделен ESP32 модул, който чете Victron **Instant Readout via Bluetooth** реклами и публикува пълен енергиен snapshot в MQTT на всеки 2 секунди.

Не се изисква GPIO — само захранване и WiFi. Дръж ESP32 на 1–3 m от Victron устройствата.

## Хардуер

| Компонент | Описание |
| --------- | -------- |
| **ESP32** | ESP32 dev платка с **4 MB flash** (WiFi + BLE) |
| **Захранване** | 5 V USB или 3.3 V стабилизирано |
| **GPIO** | Няма — само BLE модул |

Модулът е отделен от модулите 1–5, за да не се пречи BLE сканирането на релета/осветление.

BLE + WiFi изискват по-голяма flash partition (~1.5 MB firmware). В `platformio.ini` е зададено `huge_app.csv` (3 MB app на 4 MB flash).

## Настройка на Victron

За всяко устройство (SmartShunt, MPPT, Orion XS, AC зарядно по-късно):

1. Отвори **VictronConnect** на телефона.
2. **Settings → Product Info → Instant Readout via Bluetooth**.
3. Включи Instant Readout и запиши **encryption key** (32 hex символа).
4. Запиши **Bluetooth MAC** адреса.
5. Добави MAC + key в `src/Config.h`.

Устройствата рекламират на ~**200 ms**, когато Instant Readout е включен.

### Конфигурирани устройства

| JSON ключ | Устройство | Record type | MAC |
| --------- | ---------- | ----------- | --- |
| `smartshunt` | SmartShunt | `0x02` Battery Monitor | `E7:47:43:C9:5D:09` |
| `orion` | Orion XS | `0x0F` Orion XS | `E8:42:AE:38:C1:C6` |
| `mppt1` | MPPT (група панели 1) | `0x01` Solar Charger | `D3:AD:2A:CC:47:8C` |
| `mppt2` | MPPT (група панели 2) | `0x01` Solar Charger | `DC:41:88:BE:96:18` |
| `acCharger` | AC зарядно (бъдеще) | TBD | Още не е конфигурирано |

Когато имаш credentials за AC зарядното: `AC_CHARGER_ENABLED true` + `AC_CHARGER_MAC` / `AC_CHARGER_KEY`.

## Мрежа

- **WiFi SSID**: `SmartCamper`
- **WiFi парола**: `12344321`
- **MQTT Broker IP**: `192.168.4.1` (Raspberry Pi)
- **MQTT порт**: `1883`
- **Module ID**: `module-6`
- **MQTT buffer**: 1024 bytes

## MQTT Topics

### Публикувани

| Topic | Формат | Честота |
| ----- | ------ | ------- |
| `smartcamper/sensors/module-6/status` | Victron energy JSON | На всеки 2 s + при reconnect / `force_update` |
| `smartcamper/heartbeat/module-6` | Стандартен heartbeat | На всеки 10 s |

### Абонирани

| Topic | Payload | Действие |
| ----- | ------- | -------- |
| `smartcamper/commands/module-6/force_update` | `{}` | Незабавен publish |

## JSON схема

Пълен snapshot при всеки publish. Устройства без данни → `null`. След първи BLE пакет се пазят последните стойности.

```json
{
  "publishedAt": 45230,
  "smartshunt": {
    "voltage": 13.9,
    "current": 7.31,
    "soc": 99,
    "consumedAh": -2.4,
    "timeToGoMin": null,
    "alarmReason": 0,
    "updatedAt": 45100
  },
  "mppt1": { "...": "..." },
  "mppt2": { "...": "..." },
  "orion": { "...": "..." },
  "acCharger": null
}
```

### Закръгляне

| Поле | Единица | Точност |
| ---- | ------- | ------- |
| Напрежения (V) | V | 1 десетичен |
| Токове (A) | A | 2 десетични |
| SOC | % | цяло число |
| PV мощност | W | цяло число |
| yieldTodayKwh | kWh | 2 десетични |

### Остарели данни

ESP32 **не** нулира кеша. Frontend/backend маркира stale устройство когато:

```
(publishedAt - updatedAt) > 6000 ms
```

Модул offline → липсва heartbeat.

### Физическо mapping (за frontend по-късно)

- **Соларни панели 1/2**: `mppt1.pvPower` / `mppt2.pvPower` (W)
- **MPPT → батерия**: `batteryCurrent` (A)
- **Център батерия**: SmartShunt `voltage`, `current`, `soc`
- **DC натоварвания** (frontend):

  `I_dcLoads = mppt1.batteryCurrent + mppt2.batteryCurrent + orion.outputCurrent + acCharger.current − smartshunt.current`

## Инсталация

```bash
cd esp32-modules/module-6
pio run --target upload
pio device monitor
```

### Тест с mosquitto

```bash
mosquitto_sub -h 192.168.4.1 -t 'smartcamper/sensors/module-6/status' -v
mosquitto_pub -h 192.168.4.1 -t 'smartcamper/commands/module-6/force_update' -m '{}'
```

## Troubleshooting

| Проблем | Решение |
| ------- | ------- |
| `null` в JSON | Instant Readout, MAC/key, разстояние 1–3 m |
| Няма MQTT | IP в Config.h, WiFi, Mosquitto на Pi |
| BLE нестабилен | Стабилно 5 V, отделен модул, по-малко WiFi натоварване |

## Архитектура

- **ModuleManager** — WiFi, MQTT, heartbeat, commands
- **VictronManager** — BLE scan, cache, publish timer
- **VictronBleParser** — decrypt + parsers
- **CommandHandler** — `force_update`

## Debug (`src/Config.h`)

- `DEBUG_SERIAL` — serial логове (default: `true`)
- `DEBUG_MQTT` — MQTT payload логове
- `DEBUG_VERBOSE` — лог при всеки BLE update
