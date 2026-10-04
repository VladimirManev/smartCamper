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

## Link strategy (important)

GATT **connect beeps** on the fridge. Background keep-alive reconnects are therefore forbidden.

| Path | Behavior |
|------|----------|
| Status | Long-lived GATT + periodic status query / notify. Readings stay live while notify works. |
| Commands | On UI/button command, ESP **probes** write path (query must get a notify). If probe fails or there is no GATT → **one** reconnect, then apply setpoint/mode (verify + one retry). |
| Victron scan | Normal Instant Readout cadence. Paused only around fridge **commands** / write probes (`blockScanFor`), not around every status query. |

`writeValue()` has no success return — a silent write-path death can leave notify working while setpoints/mode no longer stick. On-demand probe + reconnect fixes that without random night-time beeps.

## Frontend UI (`FridgeModalContent`)

- Zone setpoints: tap badge → +/−; **1.5 s** idle debounce (resets on each step) → one absolute `zoneN/set` command.
- While waiting for BLE probe/reconnect/confirm: **spinner** (“Updating…”).
- Success when live status matches the pending setpoint/mode; failure after **~8 s** → “Connection failed”, then show current status again.
- ECO/MAX use the same pending/confirm pattern.
- Status carousel slide is read-only (temps / OFF when power relay is off).

## Notes

- One BLE client at a time — close Car Fridge Freezer / nRF Connect.
- Keep ESP32 within ~1–3 m of the fridge and Victron devices.
- Optional GPIO test buttons: see `FRIDGE_BTN_*` in `Config.h` (also use on-demand `ensureWritable()`).

## Refs

- https://github.com/dandwhelan/Alpicool50l12vfridgefreezer/blob/main/PROTOCOL.md
