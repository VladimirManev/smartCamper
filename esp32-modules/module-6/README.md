# Module-6: Victron BLE Energy Monitor + Fridge

Dedicated ESP32 module that:

1. Reads Victron **Instant Readout via Bluetooth** advertisements and publishes a full energy snapshot to MQTT every 2 seconds.
2. Connects to the **AAOBOSI / Alpicool** compressor fridge over BLE GATT (mode + dual-zone setpoints). Fridge **power** stays on module-5 relay 2.

Keep the ESP32 within 1–3 m of the Victron devices **and** the fridge. Close the phone fridge app while this module is connected (one BLE client only).

See `FRIDGE_BLE.md` for fridge protocol details and MQTT command topics.

## Hardware

| Component | Description |
| --------- | ----------- |
| **ESP32** | Any ESP32 dev board with **4 MB flash** (WiFi + BLE) |
| **Power** | 5 V USB or 3.3 V regulated supply |
| **GPIO** | Optional fridge test buttons (see `Config.h`); not required for normal use |

This module is separate from modules 1–5 so BLE does not interfere with time-critical relay/lighting logic.

BLE + WiFi need a larger flash partition than other modules (~1.5 MB firmware). `platformio.ini` uses `huge_app.csv` (3 MB app slot on 4 MB flash).

## Victron Setup

For each device (SmartShunt, MPPT, Orion XS, AC charger):

1. Open **VictronConnect** on your phone.
2. Go to **Settings → Product Info → Instant Readout via Bluetooth**.
3. Enable Instant Readout and note the **encryption key** (32 hex characters).
4. Note the device **Bluetooth MAC address** from the same screen or from a BLE scanner.
5. Add MAC + key to `src/Config.h`.

Devices advertise roughly every **200 ms** when Instant Readout is enabled.

### Configured Devices

| JSON key | Device | Record type | MAC |
| -------- | ------ | ----------- | --- |
| `smartshunt` | SmartShunt | `0x02` Battery Monitor | `E7:47:43:C9:5D:09` |
| `orion` | Orion XS | `0x0F` Orion XS | `E8:42:AE:38:C1:C6` |
| `mppt1` | MPPT (panel group 1) | `0x01` Solar Charger | `D3:AD:2A:CC:47:8C` |
| `mppt2` | MPPT (panel group 2) | `0x01` Solar Charger | `DC:41:88:BE:96:18` |
| `acCharger` | Blue Smart / Phoenix AC charger | `0x08` AC Charger | `CF:82:A4:8F:EA:04` |

`AC_CHARGER_ENABLED` is `true` in `Config.h`. Set it to `false` to drop the AC charger from the BLE device list (JSON then always has `"acCharger": null`).

## Network Configuration

- **WiFi SSID**: `SmartCamper`
- **WiFi Password**: `12344321`
- **MQTT Broker IP**: `192.168.4.1` (Raspberry Pi)
- **MQTT Broker Port**: `1883`
- **Module ID**: `module-6`
- **MQTT Buffer Size**: 1024 bytes

## MQTT Topics

### Published

| Topic | Format | Frequency |
| ----- | ------ | --------- |
| `smartcamper/sensors/module-6/status` | Victron energy JSON (see below) | Every 2 seconds + on reconnect / `force_update` |
| `smartcamper/sensors/module-6/fridge` | Fridge BLE JSON (see `FRIDGE_BLE.md`) | On status notify / reconnect / `force_update` |
| `smartcamper/heartbeat/module-6` | Standard heartbeat JSON | Every 10 seconds |

### Subscribed

| Topic | Payload | Action |
| ----- | ------- | ------ |
| `smartcamper/commands/module-6/force_update` | `{}` | Publish Victron + fridge status immediately |
| `smartcamper/commands/module-6/fridge/mode/eco` | `{}` | Set ECO mode |
| `smartcamper/commands/module-6/fridge/mode/max` | `{}` | Set MAX mode |
| `smartcamper/commands/module-6/fridge/mode/toggle` | `{}` | Toggle ECO/MAX |
| `smartcamper/commands/module-6/fridge/zone1/set` | `{"temp":5}` | Set right compartment setpoint (°C) |
| `smartcamper/commands/module-6/fridge/zone2/set` | `{"temp":-18}` | Set left (colder) compartment setpoint (°C) |

## Status Payload Schema

Full snapshot on every publish. Devices without data yet are `null`. After the first BLE packet, last known values are kept until a new packet arrives.

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
  "mppt1": {
    "deviceState": 3,
    "errorCode": 0,
    "batteryVoltage": 13.9,
    "batteryCurrent": 4.30,
    "pvPower": 62,
    "yieldTodayKwh": 0.22,
    "updatedAt": 45080
  },
  "mppt2": { "...": "..." },
  "orion": {
    "deviceState": 0,
    "errorCode": 0,
    "outputVoltage": 13.8,
    "outputCurrent": 0.00,
    "inputVoltage": 12.5,
    "inputCurrent": 0.00,
    "offReason": 129,
    "updatedAt": 45120
  },
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

`acCharger` is `null` until the first Instant Readout packet (or while shore power / charger BLE is off).

### Field Notes

| Field | Unit | Rounding |
| ----- | ---- | -------- |
| `voltage`, `batteryVoltage`, `outputVoltage`, `inputVoltage` | V | 1 decimal |
| `current`, `batteryCurrent`, `outputCurrent`, `inputCurrent`, `acCurrent` | A | 2 decimals |
| `soc` | % | integer |
| `pvPower` | W | integer |
| `yieldTodayKwh` | kWh | 2 decimals |
| `consumedAh` | Ah | 1 decimal |
| `timeToGoMin` | minutes | integer, or `null` if unavailable |
| `temperature` | °C | 1 decimal, or `null` if aux is not temperature / unavailable |
| `offReason` | hex bitmask | integer (e.g. `129` = `0x81`, normal when engine off) |
| `acCharger.voltage` / `current` | battery-side V / A from charger | 1 / 2 decimals |
| `acCharger.acCurrent` | AC input current (A) | 2 decimals |
| `updatedAt` | ms since ESP boot | set when a new BLE packet is received |
| `publishedAt` | ms since ESP boot | set at MQTT publish time |

### Stale Data (frontend / backend)

The ESP32 **does not** clear cached values when a device stops advertising. It always sends the last known values with their `updatedAt` timestamp.

Consumers should mark a device stale when:

```
(publishedAt - updatedAt) > 6000 ms
```

Module offline when heartbeat stops (standard module heartbeat logic).

### Physical Mapping (frontend energy diagram)

Mapped in `frontend/src/utils/victronToBatterySystem.js`:

- **Solar panels (group 1 / 2)**: `mppt1.pvPower` / `mppt2.pvPower` (W)
- **MPPT → battery**: `batteryCurrent` (A)
- **Battery center**: SmartShunt `voltage`, `current`, `soc`, `temperature`
- **230 V charger**: `acCharger` → UI `charger230v` (`current`, `acCurrent`)
- **Boiler UI**: a **live** (non-stale) `acCharger` also counts as shore 230 V, so the frontend can enable the boiler without the inverter (see module-5 README → Boiler ↔ 230 V)
- **DC loads** (calculated on frontend):

  `I_dcLoads = mppt1.batteryCurrent + mppt2.batteryCurrent + orion.outputCurrent + acCharger.current − smartshunt.current`

## Installation

```bash
cd esp32-modules/module-6
pio run --target upload
pio device monitor
```

### Test MQTT output

On the Pi or any machine on the camper network:

```bash
mosquitto_sub -h 192.168.4.1 -t 'smartcamper/sensors/module-6/status' -v
mosquitto_sub -h 192.168.4.1 -t 'smartcamper/sensors/module-6/fridge' -v
mosquitto_sub -h 192.168.4.1 -t 'smartcamper/heartbeat/module-6' -v
```

Force an immediate publish:

```bash
mosquitto_pub -h 192.168.4.1 -t 'smartcamper/commands/module-6/force_update' -m '{}'
```

## Troubleshooting

### No device data (`null` in JSON)

- Confirm Instant Readout is enabled in VictronConnect for each device.
- Check MAC address and encryption key in `Config.h`.
- Move ESP32 closer (1–3 m).
- Watch serial output with `DEBUG_SERIAL true`.

### MQTT not connecting

- Verify broker IP in `Config.h`.
- Check WiFi connection and serial logs.
- Ensure Mosquitto is running on the Pi.

### BLE + WiFi instability

- This module is dedicated to BLE for that reason.
- Avoid long USB cables; use a stable 5 V supply.
- Reduce WiFi traffic from other clients if scans drop packets.

## Architecture

- **ModuleManager**: WiFi, MQTT, heartbeat, commands
- **VictronManager**: BLE scan, per-device cache, JSON publish timer (`blockScanFor` only around fridge commands / write probes)
- **VictronBleParser**: AES-128-CTR decrypt + Victron record parsers
- **FridgeManager**: Alpicool GATT (service `1234`); long-lived status link; **on-demand** reconnect only before commands (fridge beeps on connect — see `FRIDGE_BLE.md`)
- **CommandHandler**: `force_update` + fridge MQTT commands

## Debug Flags (`src/Config.h`)

| Flag | Default | Purpose |
| ---- | ------- | ------- |
| `DEBUG_SERIAL` | `true` | Serial console output |
| `DEBUG_MQTT` | `false` | Log published MQTT payloads |
| `DEBUG_VERBOSE` | `false` | Log each BLE device update |

Set `DEBUG_SERIAL` to `false` for quieter production serial output.
