# Module-4: Heating Control — Dampers, Table & Autoterm UART

ESP32 module for air-distribution dampers, table lift, and (planned) Autoterm Air 2D serial bridge on the same board — one Wi‑Fi client on the Pi SoftAP.

## Overview

- **Dampers** — 5× SG90 servos + toggle buttons. Angles 0° / 45° / 90° via MQTT or local buttons (works offline).
- **Table lift** — 2 relays (up/down) + 2 buttons. Hold-to-move, double-click auto-move, MQTT commands.
- **Autoterm Air 2D** — UART bridge on GPIO **34 / 27 / 32 / 33** plus MQTT by-power control (start / set_power / stop). Thermostat and vent modes are observe-only (STOP allowed).

## Hardware Configuration

### Pin Assignment

Classic ESP32 DevKit (USB at bottom). Damper/table pins unchanged from the previous install.

| Component              | Pin | Type   | Notes |
| ---------------------- | --- | ------ | ----- |
| Damper 0 servo (Front) | 26  | Output | SG90 signal |
| Damper 0 button        | 4   | Input  | INPUT_PULLUP |
| Damper 1 servo (Rear)  | 5   | Output | |
| Damper 1 button        | 16  | Input  | |
| Damper 2 servo (Bath)  | 12  | Output | |
| Damper 2 button        | 17  | Input  | |
| Damper 3 servo (Shoes) | 13  | Output | |
| Damper 3 button        | 18  | Input  | |
| Damper 4 servo (Cockpit)| 14 | Output | |
| Damper 4 button        | 19  | Input  | |
| Table relay up         | 21  | Output | NO → + |
| Table relay down       | 22  | Output | NO → + |
| Table button up        | 23  | Input  | INPUT_PULLUP |
| Table button down      | 25  | Input  | INPUT_PULLUP |
| Autoterm panel RX      | 34  | Input  | Left header, input-only |
| Autoterm panel TX      | 27  | Output | Left header |
| Autoterm heater RX     | 32  | Input  | Left header |
| Autoterm heater TX     | 33  | Output | Left header |

Defines in `src/Config.h`: `DAMPER_*`, `TABLE_*`, `AUTOTERM_*`.

### Damper Names

| Index | Name    | Location     |
| ----- | ------- | ------------ |
| 0     | Front   | Front area   |
| 1     | Rear    | Rear area    |
| 2     | Bath    | Bathroom     |
| 3     | Shoes   | Shoe cabinet |
| 4     | Cockpit | Driver cabin |

### Servo Wiring (SG90)

- Red → 5V
- Brown/Black → GND
- Yellow/Orange → servo GPIO

### Button Wiring

- One side → GPIO, other → GND, `INPUT_PULLUP` on the ESP.

### Autoterm UART (ADuM1201)

Direct panel↔heater TX/RX wires are cut. ESP reconnects them through two ADuM1201 isolators. Same layout as the lab sniffer (validated: bridge + vent inject).

```
Comfort Control TX ---- ADuM#1 VIA (7) ---- VOA (2) ---- ESP32 GPIO 34 RX
Comfort Control RX ---- ADuM#1 VOB (6) ---- VIB (3) ---- ESP32 GPIO 27 TX

Air 2D TX ------------- ADuM#2 VIA (7) ---- VOA (2) ---- ESP32 GPIO 32 RX
Air 2D RX ------------- ADuM#2 VOB (6) ---- VIB (3) ---- ESP32 GPIO 33 TX
```

| Port   | UART  | RX | TX |
| ------ | ----- | -- | -- |
| Panel  | UART1 | 34 | 27 |
| Heater | UART2 | 32 | 33 |

- Baud: **9600 8N1** (`AUTOTERM_BAUD`)
- Panel/heater supply stay on their own +12 V / GND
- ADuM: VDD1 = 3.3 V (ESP), VDD2 = ~5 V, common GND with ESP (or isolated VDD2 if you use that scheme)
- Fuel pump / fan / glow plug / safety stay in the original ECU
- MQTT status: `smartcamper/sensors/module-4/autoterm/status`
- MQTT commands: `.../autoterm/start|set_power|stop` (by-power only; STOP always)
- Optional USB decode log: `AUTOTERM_LOG_DECODE` in `Config.h`

## Architecture

- **ModuleManager** — Network, MQTT, Heartbeat, Commands
- **DamperManager** / **DamperController** / **ServoController**
- **TableManager** / **TableController**
- **SensorManager** — coordinates dampers + table
- **CommandHandler**, **NetworkManager**, **MQTTManager**, **HeartbeatManager**
- **AutotermBridge** — panel ↔ heater bridge, status parse, by-power inject

## Configuration

Edit `src/Config.h`:

- WiFi / MQTT broker
- `NUM_DAMPERS` (5)
- Damper servo/button pins
- Table relay/button pins and timing:
  - `TABLE_DOUBLE_CLICK_TIMEOUT` — 500 ms
  - `TABLE_AUTO_MOVE_DURATION` — **20000 ms** (double-click / auto move)
  - `TABLE_START_DELAY` — 600 ms
- Autoterm: `AUTOTERM_PANEL_RX_PIN` 34, `AUTOTERM_PANEL_TX_PIN` 27, `AUTOTERM_HEATER_RX_PIN` 32, `AUTOTERM_HEATER_TX_PIN` 33, `AUTOTERM_BAUD` 9600

## Building and Uploading

```bash
cd esp32-modules/module-4
pio run -t upload
pio device monitor
```

## MQTT Topics

### Published (Status)

| Topic | Message | When |
| ----- | ------- | ---- |
| `smartcamper/sensors/module-4/damper/{index}/angle` | `{"angle": 45}` | When servo reaches target |
| `smartcamper/sensors/module-4/table/direction` | `{"direction":"up","autoMoving":true}` | On change (`up` / `down` / `stopped`) |
| `smartcamper/sensors/module-4/autoterm/status` | `{"state":"running","mode":"by-power","power":5,"errorCode":0,"heaterTemp":45,"battery":13.4,"controlLocked":false}` | On change; `error` string when `errorCode` ≠ 0 |

### Subscribed (Commands)

| Topic | Payload | Action |
| ----- | ------- | ------ |
| `smartcamper/commands/module-4/damper/{index}/set_angle` | `{"type":"damper","index":0,"action":"set_angle","angle":45}` | Set angle 0–180°, index 0–4 |
| `smartcamper/commands/module-4/table/move_up` | `{"type":"table","action":"move_up"}` | Hold-to-move up |
| `smartcamper/commands/module-4/table/move_down` | `{"type":"table","action":"move_down"}` | Hold-to-move down |
| `smartcamper/commands/module-4/table/stop` | `{"type":"table","action":"stop"}` | Stop |
| `smartcamper/commands/module-4/table/move_up_auto` | `{"type":"table","action":"move_up_auto"}` | Auto up for `TABLE_AUTO_MOVE_DURATION` (20 s) |
| `smartcamper/commands/module-4/table/move_down_auto` | `{"type":"table","action":"move_down_auto"}` | Auto down for 20 s |
| `smartcamper/commands/module-4/autoterm/start` | `{"type":"autoterm","action":"start","power":5}` | START by-power (refused if vent/thermostat active) |
| `smartcamper/commands/module-4/autoterm/set_power` | `{"type":"autoterm","action":"set_power","power":7}` | Change power while by-power running |
| `smartcamper/commands/module-4/autoterm/stop` | `{"type":"autoterm","action":"stop"}` | STOP (any mode) |
| `smartcamper/commands/module-4/force_update` | `{}` | Republish damper + table + autoterm status |

### Heartbeat

| Topic | Message | Interval |
| ----- | ------- | -------- |
| `smartcamper/heartbeat/module-4` | `{"timestamp":…,"moduleId":"module-4","uptime":…,"wifiRSSI":…}` | 10 s |

## Damper Operation

- **Button click** — cycle 0° → 45° → 90° → 0° (50 ms debounce, works offline)
- **Servo** — smooth move (~2–3 s for 0–90°), 5° steps / 150 ms, range 0–180°
- **Positions** — 0° closed, 45° half, 90° open
- **Safety** — at least one damper at 90°, or at least two at ≥45°, before closing further

## Table Lift Control

### Physical buttons

- **Hold** — move while held
- **Double-click** (within 500 ms) — auto move for **20 s** (`TABLE_AUTO_MOVE_DURATION`)
- Any button during auto-move stops immediately
- 600 ms start delay (double-click window), 50 ms debounce
- Works offline

### Frontend / MQTT

- UI single click → auto move (20 s); click again → stop
- `move_up` / `move_down` = hold-to-move; `*_auto` = timed; `stop` = halt
- Up and down relays never active together

## Installation & Setup

1. Upload firmware (`pio run -t upload`)
2. Serial monitor — WiFi, MQTT, damper/table logs
3. Dashboard — damper UI, heartbeat (blue online / red offline), local buttons offline

## Troubleshooting

### No WiFi

- SSID/password in `src/Config.h`, SoftAP `SmartCamper` up, serial errors

### No MQTT

- Broker IP `192.168.4.1`, Pi/MQTT running, ping from module network

### Servo not moving

- VCC / GND / signal on the pin from the table above (not GPIO 2)
- 5 V supply to servo, valid angle 0–180°, serial logs

### Button not working

- GPIO → button → GND, not stuck; works without network

### Autoterm

- Comfort Control “no link” → check ADuM TX path (VIB→VOB), common GND, 3.3 V / 5 V on ADuM
- Confirm baud 9600; panel frames `AA 03`, heater `AA 04`
- Boot log should show `Autoterm bridge ready`
- For verbose frame log set `AUTOTERM_LOG_DECODE true` and reflash

## Offline Operation

Dampers and table buttons work without WiFi/MQTT. Status publishes after reconnect. Auto-reconnect every few seconds.

## Status

- ✅ WiFi, MQTT, heartbeat, commands
- ✅ 5 dampers + buttons + safety rule
- ✅ Table lift (relays, buttons, MQTT)
- ✅ Autoterm bridge + by-power MQTT (34/27/32/33)
- ⏳ Cabin thermostat algorithm (own sensor) — later
