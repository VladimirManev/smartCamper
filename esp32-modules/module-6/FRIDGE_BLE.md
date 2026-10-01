# AAOBOSI / Alpicool fridge BLE (module-6)

Production home: `esp32-modules/module-6` (Victron Instant Readout + fridge GATT on one ESP32).

**Stack:** Arduino BLE (Bluedroid).  
**Fridge:** Connects by `FRIDGE_BLE_MAC` in `src/Config.h` (Victron keeps scan callbacks).  
**Power:** Module-5 relay 2 (UI Fridge card long-press / modal ON-OFF). BLE controls mode + setpoints only.

## MQTT

**Status** `smartcamper/sensors/module-6/fridge`

```json
{
  "connected": true,
  "mode": "eco",
  "zone1": { "temp": 1, "setpoint": 6 },
  "zone2": { "temp": 6, "setpoint": 6 },
  "updatedAt": 12345
}
```

UI mapping: **Left** = colder = zone2, **Right** = warmer = zone1.

**Commands**

| Topic | Payload |
|-------|---------|
| `…/commands/module-6/fridge/mode/eco` | `{}` |
| `…/commands/module-6/fridge/mode/max` | `{}` |
| `…/commands/module-6/fridge/mode/toggle` | `{}` |
| `…/commands/module-6/fridge/zone1/set` | `{"temp":5}` |
| `…/commands/module-6/fridge/zone2/set` | `{"temp":-18}` |

## Device

| Field | Value |
|-------|-------|
| BLE name | `A1-222A06A6F348` |
| MAC | `22:2a:06:a6:f3:48` |
| Service | `1234` / write `1235` / notify `1236` |

## Notes

- One BLE client at a time — close Car Fridge Freezer / nRF Connect.
- Keep ESP32 within ~1–3 m of the fridge and Victron devices.
- Optional GPIO test buttons: see `FRIDGE_BTN_*` in `Config.h`.

## Refs

- https://github.com/dandwhelan/Alpicool50l12vfridgefreezer/blob/main/PROTOCOL.md
