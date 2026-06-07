# Module-7: Сензор за ниво на чиста вода

Отделен ESP32 модул за измерване на нивото в резервоара за чиста (питейна) вода чрез проводими електроди.

Същата логика като сивата вода в module-1 — отделен модул, за да не споделя GPIO с други сензори.

## Хардуер

| Компонент | Описание |
| --------- | -------- |
| **ESP32** | ESP32 dev платка |
| **Захранване** | 5 V USB или 3.3 V стабилизирано |
| **Електроди** | 7 проводими сонди в резервоара за чиста вода |

### GPIO свързване (отдолу → отгоре)

| GPIO | Ниво |
| ---- | ---- |
| 4 | 15% |
| 5 | 30% |
| 18 | 45% |
| 19 | 60% |
| 21 | 75% |
| 22 | 90% |
| 23 | 100% |

Всички електроди споделят общ GND в резервоара. Всеки GPIO пин е свързан към един електрод.

### ESP32 ограничения

- Само **един пин с INPUT_PULLUP** по време на измерване (по-малко ток през водата → по-малко корозия).
- Между четенията пиновете са в INPUT без pull-up.
- Интервал 30 s по подразбиране — не намалявай без причина (износване на електродите).
- Избягвай GPIO 6–11 (свързани с flash на повечето ESP32 платки).

## Мрежа

- **WiFi SSID**: `SmartCamper`
- **WiFi парола**: `12344321`
- **MQTT Broker IP**: `192.168.4.1` (Raspberry Pi)
- **MQTT порт**: `1883`
- **Module ID**: `module-7`

## MQTT Topics

### Публикувани

| Topic | Формат | Честота |
| ----- | ------ | ------- |
| `smartcamper/sensors/clean-water/level` | Число `0`–`100` (процент) | При промяна (≥1%), reconnect или `force_update` |
| `smartcamper/heartbeat/module-7` | Стандартен heartbeat JSON | На всеки 10 s |

### Абонирани

| Topic | Payload | Действие |
| ----- | ------- | -------- |
| `smartcamper/commands/module-7/force_update` | `{}` | Незабавен publish (последно единично четене) |

## Логика на измерване

1. Четене отгоре → надолу (100% → 15%).
2. Най-високият покрит електрод определя нивото.
3. Rolling window от 10 проби (~5 мин) — публикува се **модата** (най-честата стойност).
4. При MQTT reconnect или `force_update` — веднага се публикува последното единично четене.

## Конфигурация (`src/Config.h`)

| Константа | По подразбиране | Назначение |
| --------- | --------------- | ---------- |
| `CLEAN_WATER_LEVEL_READ_INTERVAL` | 30000 ms | Интервал между четения |
| `CLEAN_WATER_LEVEL_MODE_SAMPLE_COUNT` | 10 | Брой проби за mode филтъра |
| `CLEAN_WATER_LEVEL_THRESHOLD` | 1.0 | Мин. промяна в % за publish |
| `HEARTBEAT_INTERVAL` | 10000 ms | Heartbeat интервал |

Нива по електроди: 15, 30, 45, 60, 75, 90, 100%.

## Backend

- **Handler**: `sensorDataHandler.js` → `handleCleanWater`
- **WebSocket**: `sensorUpdate` с `{ cleanWaterLevel, timestamp }`
- **Онлайн статус**: heartbeat `module-7` → `ModuleRegistry`
- **Force refresh**: `forceModuleUpdate` с `{ moduleId: "module-7" }`

## Frontend

- **Карта в менюто**: Fresh Water — икона кран, синя течност
- **Модал**: детайлен изглед; poll на module-7 на всеки 5 s докато е отворен
- **Status панел** (tablet): в ротацията заедно с батерия, сива вода, тоалет, overview
- **Offline**: картата/модалът са disabled без heartbeat от module-7

## Инсталация

```bash
cd esp32-modules/module-7
pio run --target upload
pio device monitor
```

### Тест с mosquitto

```bash
mosquitto_sub -h 192.168.4.1 -t 'smartcamper/sensors/clean-water/level' -v
mosquitto_sub -h 192.168.4.1 -t 'smartcamper/heartbeat/module-7' -v
mosquitto_pub -h 192.168.4.1 -t 'smartcamper/commands/module-7/force_update' -m '{}'
```

## Архитектура

- **ModuleManager** — WiFi, MQTT, heartbeat, commands
- **CleanWaterLevelManager** — координира сензора и командите
- **CleanWaterLevelSensor** — електроди, mode филтър, MQTT publish
- **CommandHandler** — `force_update`

## Troubleshooting

| Проблем | Проверка |
| ------- | -------- |
| Ниво винаги 0% | Окабеляване, общ GND в резервоара, проводимост на водата |
| Няма MQTT | IP в Config.h, WiFi, Mosquitto на Pi |
| Frontend показва offline | Heartbeat, захранване, WiFi |
| Скачащо ниво | Нормално до пълнене на mode прозореца (~5 мин) |
| Корозия на електроди | Не намалявай read interval без причина |

## Debug (`src/Config.h`)

- `DEBUG_SERIAL` — serial логове (default: `true`)
- `DEBUG_MQTT` — MQTT payload логове
- `DEBUG_VERBOSE` — допълнителен детайл (не се ползва)

Пълна техническа документация (на английски): **`README.md`** в същата папка.
