# Module-7: Clean Water Level Sensor

Dedicated ESP32 module for clean (fresh) water tank level measurement using conductivity-based electrodes.

Same sensor logic as module-1 gray water — separated into its own module so level sensing does not share GPIO with other sensors.

## Hardware

| Component | Description |
| --------- | ----------- |
| **ESP32** | Any ESP32 dev board |
| **Power** | 5 V USB or 3.3 V regulated supply |
| **Electrodes** | 7 conductivity probes in the clean water tank |

### GPIO Wiring (bottom → top)

| GPIO | Level |
| ---- | ----- |
| 4 | 15% |
| 5 | 30% |
| 18 | 45% |
| 19 | 60% |
| 21 | 75% |
| 22 | 90% |
| 23 | 100% |

All electrodes share a common GND reference in the tank. Each GPIO pin connects to one electrode.

### ESP32 Constraints

- Only **one pin uses INPUT_PULLUP at a time** during measurement (reduces current through water → less corrosion).
- Pins stay in plain INPUT (no pull-up) between readings.
- 30 s read interval by default — do not lower without a good reason (electrode wear).
- Avoid GPIO 6–11 (connected to flash on most ESP32 boards).

## Network Configuration

- **WiFi SSID**: `SmartCamper`
- **WiFi Password**: `12344321`
- **MQTT Broker IP**: `192.168.4.1` (Raspberry Pi)
- **MQTT Broker Port**: `1883`
- **Module ID**: `module-7`

## MQTT Topics

### Published

| Topic | Message Format | Frequency |
| ----- | -------------- | --------- |
| `smartcamper/sensors/clean-water/level` | Plain number `0`–`100` (percent) | On change (≥1%), MQTT reconnect, or `force_update` |
| `smartcamper/heartbeat/module-7` | `{"timestamp":…,"moduleId":"module-7","uptime":…,"wifiRSSI":…}` | Every 10 seconds |

### Subscribed

| Topic | Payload | Action |
| ----- | ------- | ------ |
| `smartcamper/commands/module-7/force_update` | `{}` | Publish level immediately (latest single reading) |

## Measurement Logic

1. Read electrodes top → bottom (100% → 15%).
2. Highest covered electrode determines the level.
3. Rolling window of 10 samples (~5 min) — publish the **mode** (most frequent reading).
4. On MQTT reconnect or `force_update` — publish the latest single reading immediately (skip mode filter).

## Configuration (`src/Config.h`)

| Constant | Default | Purpose |
| -------- | ------- | ------- |
| `CLEAN_WATER_LEVEL_READ_INTERVAL` | `30000` ms | Time between electrode readings |
| `CLEAN_WATER_LEVEL_MODE_SAMPLE_COUNT` | `10` | Samples in rolling mode window |
| `CLEAN_WATER_LEVEL_THRESHOLD` | `1.0` | Min % change to publish (normal reads) |
| `HEARTBEAT_INTERVAL` | `10000` ms | Heartbeat publish interval |

Level percentages per electrode: 15, 30, 45, 60, 75, 90, 100.

## Backend Integration

- **MQTT handler**: `backend/socket/handlers/sensorDataHandler.js` → `handleCleanWater`
- **WebSocket event**: `sensorUpdate` with `{ cleanWaterLevel, timestamp }`
- **Module online**: `smartcamper/heartbeat/module-7` → `ModuleRegistry`
- **Force refresh**: frontend emits `forceModuleUpdate` with `{ moduleId: "module-7" }`

## Frontend Integration

- **Main menu card**: Fresh Water (`FreshWaterTank`) — faucet icon, blue liquid fill
- **Detail modal**: `FreshWaterModalContent` — polls module-7 every 5 s while open
- **Status panel** (tablet): included in rotating slides (Battery → Gray Water → Fresh Water → Toilet → Overview)
- **Offline**: card/modal disabled when `module-7` heartbeat stops

## Installation

```bash
cd esp32-modules/module-7
pio run --target upload
pio device monitor
```

### Test MQTT

```bash
mosquitto_sub -h 192.168.4.1 -t 'smartcamper/sensors/clean-water/level' -v
mosquitto_sub -h 192.168.4.1 -t 'smartcamper/heartbeat/module-7' -v
mosquitto_pub -h 192.168.4.1 -t 'smartcamper/commands/module-7/force_update' -m '{}'
```

## Architecture

- **ModuleManager** — WiFi, MQTT, heartbeat, commands
- **CleanWaterLevelManager** — Coordinates sensor and command handler
- **CleanWaterLevelSensor** — Electrode reading, mode filter, MQTT publish
- **CommandHandler** — `force_update` command

## Troubleshooting

| Problem | Check |
| ------- | ----- |
| Level stuck at 0% | Electrode wiring, shared GND in tank, water conductivity |
| No MQTT data | Broker IP in `Config.h`, WiFi, Mosquitto on Pi |
| Frontend shows offline | Heartbeat topic, module powered and on WiFi |
| Erratic level jumps | Normal until mode window fills (~5 min); wait or use `force_update` |
| Corrosion on electrodes | Do not shorten `CLEAN_WATER_LEVEL_READ_INTERVAL` without reason |

## Debug Flags (`src/Config.h`)

| Flag | Default | Purpose |
| ---- | ------- | ------- |
| `DEBUG_SERIAL` | `true` | Serial console output |
| `DEBUG_MQTT` | `false` | Log published MQTT payloads |
| `DEBUG_VERBOSE` | `false` | Extra detail (unused) |

Set `DEBUG_SERIAL` to `false` for quieter production serial output.
