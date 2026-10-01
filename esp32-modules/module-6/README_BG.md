# Module-6: Victron BLE енергиен монитор + хладилник

Отделен ESP32 модул, който:

1. Чете Victron **Instant Readout via Bluetooth** и публикува енергиен snapshot в MQTT на всеки 2 s.
2. Свързва се към **AAOBOSI / Alpicool** хладилника по BLE GATT (режим + две зони). **Захранването** остава на реле 2 на module-5.

Дръж ESP32 на 1–3 m от Victron устройствата **и** хладилника. Затвори телефонното fridge app докато модулът е свързан.

Подробности: `FRIDGE_BLE.md` и английския `README.md`.

## Хардуер

| Компонент | Описание |
| --------- | -------- |
| **ESP32** | ESP32 dev платка с **4 MB flash** (WiFi + BLE) |
| **Захранване** | 5 V USB или 3.3 V стабилизирано |
| **GPIO** | Опционални бутони за fridge тест (виж `Config.h`); не са нужни за нормална работа |

Модулът е отделен от модулите 1–5, за да не се пречи BLE сканирането на релета/осветление.

BLE + WiFi изискват по-голяма flash partition (~1.5 MB firmware). В `platformio.ini` е зададено `huge_app.csv` (3 MB app на 4 MB flash).

## Настройка на Victron

За всяко устройство (SmartShunt, MPPT, Orion XS, AC зарядно):

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
| `acCharger` | Blue Smart / Phoenix AC зарядно | `0x08` AC Charger | `CF:82:A4:8F:EA:04` |

`AC_CHARGER_ENABLED` е `true` в `Config.h`. При `false` AC зарядното се маха от BLE списъка и JSON винаги има `"acCharger": null`.

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
| `smartcamper/sensors/module-6/fridge` | Fridge BLE JSON (виж `FRIDGE_BLE.md`) | При notify / reconnect / `force_update` |
| `smartcamper/heartbeat/module-6` | Стандартен heartbeat | На всеки 10 s |

### Абонирани

| Topic | Payload | Действие |
| ----- | ------- | -------- |
| `smartcamper/commands/module-6/force_update` | `{}` | Незабавен publish на Victron + fridge |
| `smartcamper/commands/module-6/fridge/mode/eco` | `{}` | ECO режим |
| `smartcamper/commands/module-6/fridge/mode/max` | `{}` | MAX режим |
| `smartcamper/commands/module-6/fridge/mode/toggle` | `{}` | Превключи ECO/MAX |
| `smartcamper/commands/module-6/fridge/zone1/set` | `{"temp":5}` | Дясна камера (°C) |
| `smartcamper/commands/module-6/fridge/zone2/set` | `{"temp":-18}` | Лява (по-студена) камера (°C) |

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
    "temperature": 24.5,
    "alarmReason": 0,
    "updatedAt": 45100
  },
  "mppt1": { "...": "..." },
  "mppt2": { "...": "..." },
  "orion": { "...": "..." },
  "acCharger": {
    "deviceState": 3,
    "errorCode": 0,
    "voltage": 14.2,
    "current": 8.50,
    "acCurrent": 2.40,
    "updatedAt": 45110
  }
}
```

`acCharger` е `null` до първия Instant Readout пакет (или докато shore power / зарядът по BLE е изключен).

### Закръгляне

| Поле | Единица | Точност |
| ---- | ------- | ------- |
| Напрежения (V) | V | 1 десетичен |
| Токове (A), вкл. `acCurrent` | A | 2 десетични |
| SOC | % | цяло число |
| `temperature` | °C | 1 десетичен, или `null` ако aux не е температура |
| PV мощност | W | цяло число |
| yieldTodayKwh | kWh | 2 десетични |
| `acCharger.voltage` / `current` | V / A към батерията | 1 / 2 десетични |

### Остарели данни

ESP32 **не** нулира кеша. Frontend/backend маркира stale устройство когато:

```
(publishedAt - updatedAt) > 6000 ms
```

Модул offline → липсва heartbeat.

### Физическо mapping (frontend energy diagram)

Мапва се в `frontend/src/utils/victronToBatterySystem.js`:

- **Соларни панели 1/2**: `mppt1.pvPower` / `mppt2.pvPower` (W)
- **MPPT → батерия**: `batteryCurrent` (A)
- **Център батерия**: SmartShunt `voltage`, `current`, `soc`, `temperature`
- **230 V зарядно**: `acCharger` → UI `charger230v` (`current`, `acCurrent`)
- **Бойлер UI**: живо (не stale) `acCharger` се ползва като индикатор за къмпинг 230 V — frontend може да активира бойлера без инвертор (виж module-5 README_BG → Бойлер ↔ 230 V)
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
- **VictronManager** — BLE scan, cache, publish timer (`pauseScan` при fridge GATT connect)
- **VictronBleParser** — decrypt + parsers (вкл. AC charger `0x08`)
- **FridgeManager** — Alpicool GATT клиент, status + mode/zone команди
- **CommandHandler** — `force_update` + fridge MQTT команди

## Debug (`src/Config.h`)

- `DEBUG_SERIAL` — serial логове (default: `true`)
- `DEBUG_MQTT` — MQTT payload логове
- `DEBUG_VERBOSE` — лог при всеки BLE update
