# AAOBOSI / Alpicool fridge BLE (module-6)

Production home: `esp32-modules/module-6` (Victron Instant Readout + fridge GATT + **mains power relay**).

**Stack:** Arduino BLE (Bluedroid).  
**Fridge BLE:** `FRIDGE_BLE_MAC` in `src/Config.h`.  
**Power:** Local relay **GPIO 14** (moved from module-5 GPIO 16). Physical toggle **GPIO 22**. BLE controls mode + zone setpoints only while power is ON.

## MQTT

**Status** `smartcamper/sensors/module-6/fridge`

```json
{
  "connected": true,
  "powerRelay": "ON",
  "haveStatus": true,
  "mode": "eco",
  "zone1": { "temp": 1, "setpoint": 6 },
  "zone2": { "temp": 6, "setpoint": 6 },
  "updatedAt": 12345
}
```

- `powerRelay` — local mains relay (`ON` / `OFF`). Backend mirrors this to appliance UI index 2.
- `connected` — active GATT link with a parsed status frame.
- Backend adds `receivedAt` (wall-clock ms) when forwarding to the UI.

UI mapping: **Left** = colder = zone2, **Right** = warmer = zone1.

**Commands**

| Topic | Payload |
|-------|---------|
| `…/commands/module-6/fridge/power/on` | `{}` |
| `…/commands/module-6/fridge/power/off` | `{}` |
| `…/commands/module-6/fridge/power/toggle` | `{}` |
| `…/commands/module-6/fridge/mode/eco` | `{}` |
| `…/commands/module-6/fridge/mode/max` | `{}` |
| `…/commands/module-6/fridge/mode/toggle` | `{}` |
| `…/commands/module-6/fridge/zone1/set` | `{"temp":5}` |
| `…/commands/module-6/fridge/zone2/set` | `{"temp":-18}` |

Frontend fridge long-press still emits appliance relay index 2; backend routes it to `fridge/power/…`.

## Device

| Field | Value |
|-------|-------|
| BLE name | `A1-222A06A6F348` |
| MAC | `22:2a:06:a6:f3:48` |
| Service | `1234` / write `1235` / notify `1236` |
| Mains relay | GPIO **14** |
| Relay button | GPIO **22** |

## Link strategy

| Path | Behavior |
|------|----------|
| Power OFF | Drop GATT; **no** background `connect()` (blocks ~30 s and kills WiFi/MQTT/Victron). |
| Power ON | Status GATT + query every `FRIDGE_QUERY_INTERVAL_MS`; auto-reconnect with backoff if the link drops. |
| Commands | Probe write path; reconnect if needed; ATT **Write Request**; confirm query. |
| Victron scan | Brief `blockScanFor` around connect/writes; cleared on failure / power OFF. |

Writes use GATT Write Request (with response), chunked to ≤20 bytes.

**MQTT:** Fridge status is published from `loop()`, never from the BLE notify callback.

## Frontend UI

- Fridge card inactive when **module-6** is offline.
- Zone setpoints: 1.5 s debounce → absolute set; spinner until confirm (~20 s timeout).
- Age / stale (45 s) → dashes.

## Notes

- One BLE client at a time — close Car Fridge Freezer / nRF Connect.
- Keep ESP32 within ~1–3 m of the fridge and Victron devices.
- Optional BLE test buttons: `FRIDGE_BTN_*` in `Config.h` (not the mains relay).

## Refs

- https://github.com/dandwhelan/Alpicool50l12vfridgefreezer/blob/main/PROTOCOL.md
