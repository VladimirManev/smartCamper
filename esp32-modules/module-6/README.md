# Module-6: Victron BLE Energy Monitor

Dedicated ESP32 module that reads Victron **Instant Readout via Bluetooth** advertisements and publishes a full energy snapshot to MQTT every 2 seconds.

No GPIO wiring is required — only power and WiFi. Keep the ESP32 within 1–3 m of the Victron devices.

## Hardware

| Component | Description |
| --------- | ----------- |
| **ESP32** | Any ESP32 dev board with **4 MB flash** (WiFi + BLE) |
| **Power** | 5 V USB or 3.3 V regulated supply |
| **GPIO** | None — BLE-only module |

This module is separate from modules 1–5 so BLE scanning does not interfere with time-critical relay/lighting logic.

BLE + WiFi need a larger flash partition than other modules (~1.5 MB firmware). `platformio.ini` uses `huge_app.csv` (3 MB app slot on 4 MB flash).

## Victron Setup

For each device (SmartShunt, MPPT, Orion XS, AC charger later):

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
| `acCharger` | AC charger (future) | TBD | Not configured yet |

Set `AC_CHARGER_ENABLED` to `true` and fill `AC_CHARGER_MAC` / `AC_CHARGER_KEY` when credentials are available.

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
| `smartcamper/heartbeat/module-6` | Standard heartbeat JSON | Every 10 seconds |

### Subscribed

| Topic | Payload | Action |
| ----- | ------- | ------ |
| `smartcamper/commands/module-6/force_update` | `{}` | Publish status immediately |

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
  "acCharger": null
}
```

### Field Notes

| Field | Unit | Rounding |
| ----- | ---- | -------- |
| `voltage`, `batteryVoltage`, `outputVoltage`, `inputVoltage` | V | 1 decimal |
| `current`, `batteryCurrent`, `outputCurrent`, `inputCurrent` | A | 2 decimals |
| `soc` | % | integer |
| `pvPower` | W | integer |
| `yieldTodayKwh` | kWh | 2 decimals |
| `consumedAh` | Ah | 1 decimal |
| `timeToGoMin` | minutes | integer, or `null` if unavailable |
| `offReason` | hex bitmask | integer (e.g. `129` = `0x81`, normal when engine off) |
| `updatedAt` | ms since ESP boot | set when a new BLE packet is received |
| `publishedAt` | ms since ESP boot | set at MQTT publish time |

### Stale Data (frontend / backend)

The ESP32 **does not** clear cached values when a device stops advertising. It always sends the last known values with their `updatedAt` timestamp.

Consumers should mark a device stale when:

```
(publishedAt - updatedAt) > 6000 ms
```

Module offline when heartbeat stops (standard module heartbeat logic).

### Physical Mapping (for future frontend)

- **Solar panels (group 1 / 2)**: show `mppt1.pvPower` / `mppt2.pvPower` in watts.
- **MPPT → battery wire**: `batteryCurrent` in amps.
- **Battery center**: SmartShunt `voltage`, `current`, `soc`.
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
- **VictronManager**: BLE scan, per-device cache, JSON publish timer
- **VictronBleParser**: AES-128-CTR decrypt + Victron record parsers
- **CommandHandler**: `force_update` command

## Debug Flags (`src/Config.h`)

| Flag | Default | Purpose |
| ---- | ------- | ------- |
| `DEBUG_SERIAL` | `true` | Serial console output |
| `DEBUG_MQTT` | `false` | Log published MQTT payloads |
| `DEBUG_VERBOSE` | `false` | Log each BLE device update |

Set `DEBUG_SERIAL` to `false` for quieter production serial output.
