# Module-8: Security Alarm

Autonomous ESP32 alarm for SmartCamper: interior zone (siren + smoke), perimeter zone (buzzer only), MQTT status/commands.

Works offline (button + local logic). WiFi/MQTT follow the same pattern as other modules.

## Hardware (GPIO)

| Function | Pin | Notes |
|----------|-----|--------|
| BOOT button | 0 | Built-in; sequences below |
| Spare contact | 4 | `INPUT_PULLUP`, LOW = open (hood / door sim) |
| Interior PIR | 5 | HIGH = motion |
| Perimeter PIR front | 18 | HIGH = motion |
| Perimeter PIR rear | 19 | |
| Perimeter PIR left_front | 21 | |
| Perimeter PIR left_rear | 22 | |
| Perimeter PIR right_front | 23 | |
| Perimeter PIR right_rear | 25 | |
| Buzzer | 26 | Active HIGH |
| Siren relay | 27 | Active HIGH |
| Smoke relay | 32 | Active HIGH |
| Zone1 status LED | 33 | Blinks 500 ms when zone 1 arming/armed |

Adjust pins in `src/Config.h` if your board wiring differs.

## Button sequences

| Sequence | Action |
|----------|--------|
| 1 short + hold ≥3 s | Arm zone 1 **normal**, or **disarm** zone 1 (normal or cat) |
| 2 short + hold ≥3 s | Toggle zone 2 (perimeter) |
| 3 short + hold ≥3 s | Arm zone 1 **cat mode** only (ignore interior PIR) |
| Other | Long error beep |

Timing: short &lt; 500 ms; gap between taps ≤ 800 ms; hold ≥ 3 s.

While zone 1 is armed or arming, the cat sequence beeps error — disarm with **1 short + hold**.

## Zone 1 behaviour

1. Confirmation beeps → 30 s exit delay (sensors ignored; escalated buzzer) → ARMED  
2. Trip (spare open edge / interior PIR if not cat) → 30 s entry delay → siren 2 min  
3. Smoke starts 10 s after siren, runs 1 min  
4. After 2 min: outputs off, stays ARMED; new motion or new spare edge → new entry delay  
5. Already-open spare after an alarm cycle is ignored until it closes

## Zone 2 behaviour

Arms immediately after confirmation. Motion → 5 short beeps; repeats every 8 s while motion continues. Suppressed while zone 1 delay buzzer is playing.

## Network

- WiFi: `SmartCamper` / `12344321`
- MQTT: `192.168.4.1:1883`
- Module ID: `module-8`
- Disarm PIN (MQTT zone 1 only): `1234` in `Config.h` (reflash to change)

## MQTT

### Publish

| Topic | When |
|-------|------|
| `smartcamper/heartbeat/module-8` | Every 10 s |
| `smartcamper/sensors/module-8/status` | On any change + `force_update` |

Status JSON includes `zone1` (armed, phase, ignoreInteriorPir, siren, smoke), `zone2`, `inputs` (spareOpen, interiorPir, doors stub, perimeter).

`phase`: `idle` | `exit_delay` | `armed` | `entry_delay` | `alarm`  
Doors are stubbed `false` until CAN integration.

### Subscribe

| Topic | Payload |
|-------|---------|
| `.../zone/1/on` | `{}` or `{"ignoreInteriorPir":true}` |
| `.../zone/1/off` | `{"pin":"1234"}` |
| `.../zone/2/on` | `{}` |
| `.../zone/2/off` | `{}` (no PIN) |
| `.../force_update` | `{}` |

## Build / flash

```bash
cd esp32-modules/module-8
pio run -t upload
pio device monitor
```

## Out of scope (later)

- Fiat Ducato B-CAN door parsing (SN65HVD230)
- Frontend / backend Socket.io handlers
