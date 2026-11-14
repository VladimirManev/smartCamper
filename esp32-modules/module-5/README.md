# Module-5: Appliance Controller

ESP32 module for controlling six appliances via relays (Audio System, Water Pump, Refrigerator, WC Fan, Boiler, Inverter) plus a seventh relay for tablet display backlight.

## Hardware Configuration

### Pin Assignment

| Component                   | Pin | Type            | Description                    |
| --------------------------- | --- | --------------- | ------------------------------ |
| **Relay 0** (Audio System)  | 14  | Output          | Relay control for audio        |
| **Relay 1** (Water Pump)    | 15  | Output          | Relay control for pump         |
| **Relay 2** (Refrigerator)  | —   | —               | **Moved to module-6** (GPIO 14) |
| **Relay 3** (WC Fan)        | 23  | Output          | Relay control for WC fan       |
| **Relay 4** (Boiler)        | 27  | Output          | Relay control for boiler       |
| **Relay 5** (Inverter)      | 32  | Output          | Relay control for inverter     |
| **Relay 6** (Display backlight) | 13 | Output       | Tablet backlight (NO contact)  |
| **Button 0** (Audio System) | 17  | Input (Pull-up) | Manual toggle for relay 0      |
| **Button 1** (Water Pump)   | 21  | Input (Pull-up) | Manual toggle for relay 1      |
| **Button 2** (Refrigerator) | —   | —               | **Moved to module-6** (GPIO 22) |
| **Button 3** (WC Fan)       | 25  | Input (Pull-up) | Manual toggle for relay 3      |
| **Button 4** (Boiler)       | 26  | Input (Pull-up) | Manual toggle for relay 4      |
| **Button 5** (Inverter)     | 33  | Input (Pull-up) | Manual toggle for relay 5      |
| **Urine Level 50%**         | 4   | Input           | Toilet tank lower electrode    |
| **Urine Level 100%**        | 5   | Input           | Toilet tank upper electrode    |
| **Boiler electrode**        | 16  | Input           | Full-tank electrode. Pull-up only while reading. LOW = full |
| **Boiler temp bottom**      | 18  | Input           | DS18B20, thermostat |
| **Boiler temp top**         | 19  | Input           | DS18B20, display and 75°C limit |

### Appliance Details

| Relay | Name         | Relay Pin | Button Pin | Description          |
| ----- | ------------ | --------- | ---------- | -------------------- |
| **0** | Audio System | 14        | 17         | Audio system control |
| **1** | Water Pump   | 15        | 21         | Water pump control   |
| **2** | Refrigerator | —         | —          | **Moved to module-6** (power + BLE) |

### Refrigerator (moved)

Fridge **power + BLE** are on **module-6** (relay GPIO 14, button GPIO 22). See `../module-6/FRIDGE_BLE.md`. Module-5 ignores MQTT/button for relay index 2.
| **3** | WC Fan       | 23        | 25         | WC fan control       |
| **4** | Boiler       | 27        | 26         | Boiler control       |
| **5** | Inverter     | 32        | 33         | Inverter control     |
| **6** | Display backlight | 13 | — | Tablet UI — no physical button |

### Relay 6 — tablet display backlight

- **GPIO 13**, **NO contact** in series with backlight supply (HIGH = relay energized = backlight ON)
- Boot default: backlight **ON**
- Frontend only (tablet landscape): long-press clock to off, Settings auto-off, tap black overlay to wake
- **Not** shown on appliance cards in the UI
- Display physical button may be wired in parallel across NO (bypass relay)

### Button Functions

- **Click**: Toggle relay ON/OFF
- Simple toggle behavior (no dimming or hold)

## Network Configuration

- **WiFi SSID**: `SmartCamper`
- **WiFi Password**: `12344321`
- **MQTT Broker IP**: `192.168.4.1` (Raspberry Pi)
- **MQTT Broker Port**: `1883`
- **Module ID**: `module-5`
- **MQTT Buffer Size**: 1024 bytes

## MQTT Topics

### Published (Status)

| Topic                                 | Message Format                                                               | Update Frequency                                          |
| ------------------------------------- | ---------------------------------------------------------------------------- | --------------------------------------------------------- |
| `smartcamper/sensors/module-5/status` | `{"relays": {"0": {"state": "ON"}, ...}}`                                    | On change only (button press, MQTT command, force_update) |
| `smartcamper/sensors/toilet/urine/level` | `50` (0 / 50 / 100)                                                       | On change (≥1%) or `force_update`                         |
| `smartcamper/heartbeat/module-5`      | `{"timestamp": ..., "moduleId": "module-5", "uptime": ..., "wifiRSSI": ...}` | Every 10 seconds                                          |

### Subscribed (Commands)

| Topic                                                | Payload | Action              |
| ---------------------------------------------------- | ------- | ------------------- |
| `smartcamper/commands/module-5/relay/{index}/toggle` | `{}`    | Toggle relay        |
| `smartcamper/commands/module-5/relay/{index}/on`     | `{}`    | Relay ON. Index 4 starts maintain mode |
| `smartcamper/commands/module-5/relay/{index}/off`    | `{}`    | Relay OFF. Index 4 stops every boiler mode |
| `smartcamper/commands/module-5/boiler/setpoint`      | `{"value": 60}` | Maintain setpoint 5–70°C. Not saved across restart |
| `smartcamper/commands/module-5/boiler/once`          | `{"value": 30}` | One-shot heat to 30 or 60°C. Not saved across restart |
| `smartcamper/commands/module-5/force_update`         | `{}`    | Force status update, including one electrode read |

### Command Format

- `{index}` can be 0–6 (six appliances + display backlight)
- Example: `smartcamper/commands/module-5/relay/0/toggle` - toggles relay 0 (Audio System)
- Example: `smartcamper/commands/module-5/relay/6/off` - turns off tablet display backlight
- Example: `smartcamper/commands/module-5/relay/6/on` - turns on tablet display backlight

## Features

### Relay Control

- Each relay can be controlled independently
- Physical buttons work offline (without WiFi/MQTT)
- MQTT commands work when module is connected
- Status is published automatically on every change

### Behavior

- Same behavior as Ambient relay in module-2
- Simple toggle - no dimming or complex modes
- Button toggles relay immediately on press

### Toilet Urine Level Sensor

Two conductivity electrodes in the urine tank (same approach as module-1 gray water, fewer levels):

| GPIO | Level |
| ---- | ----- |
| 4 | 50% |
| 5 | 100% |

- Read interval: 30 s (electrode-friendly)
- Published level: **mode** over last 10 samples (~5 min), or latest reading on `force_update` / MQTT reconnect
- Possible values: **0%, 50%, 100%**
- Only one pin uses `INPUT_PULLUP` at a time during measurement

Configuration in `src/Config.h`: `URINE_LEVEL_*` constants.

### Boiler

The element relay (GPIO 27) is closed only by the boiler controller. Relay index 4 in `module-5/status` means a mode is active, not that the element is on. Element state is `relay` in the boiler topic.

| GPIO | Role |
| ---- | ---- |
| 27 | Element relay. HIGH = on. Owned by `BoilerController`, not `RelayController` |
| 26 | Button. From off, starts maintain mode. From any mode, stops the boiler |
| 16 | Electrode at the top of the tank. Water pulls the pin to GND |
| 18 | Bottom DS18B20. Maintain mode turns the element off at the setpoint and on at setpoint − 3. One-shot mode stops at 30 or 60 |
| 19 | Top DS18B20. Shown in the UI. A reading of 75°C stops the boiler. A missing top sensor does not |

DS18B20 VCC is 5 V, same as the other sensors. The 4.7 kΩ pull-up goes from data to **3.3 V**, not to 5 V. GPIO 18 and 19 also keep the ESP32 internal pull-up on, in parallel with that resistor. A 12-bit conversion takes about 750 ms. The two sensors convert at the same time, and the wait is spread across `loop()` so the module does not freeze. Every 30 s the firmware takes 3 readings and keeps the median when at least 2 are inside −10…95°C. A failed read is tried twice more immediately before that sample counts as missed. Integers are published only when they change.

The electrode is read only before the element closes, once at boot, and on `force_update` (including MQTT reconnect, a new frontend connection, and the toilet modal poll). The internal pull-up is enabled for a few milliseconds per sample, then the pin is released. Three samples, majority wins.

Two failed bursts on the bottom sensor, a reading of 75°C or more on a sensor that did answer, or no water at the moment the element should close: the element opens, the mode clears, and it stays off until the next command. A missing top sensor clears the top temperature and does not stop the boiler. One missed burst keeps the last good temperature. Setpoint defaults to 60°C after every restart, and one-shot mode is forgotten.

Maintain mode keeps reheating at setpoint − 3. One-shot mode heats until the bottom sensor reaches 30 or 60, then turns off and does not reheat. The module finishes a one-shot without the backend. Pressing the active one-shot target again, relay 4 off, or the hardware button stops it. Relay 4 on during a one-shot switches to maintain mode and keeps the stored setpoint.

Published topic `smartcamper/sensors/module-5/boiler`:

```json
{"enabled": false, "mode": "off", "once": null, "relay": false, "setpoint": 60, "top": null, "bottom": null, "full": true, "error": null}
```

`mode` is `"off"`, `"maintain"`, or `"once"`. `once` is `30`, `60`, or `null`. `enabled` is true whenever `mode` is not `"off"`.

`error` is `null`, `"empty"`, `"sensor"`, or `"overtemp"`. `full` is `null` only before the first electrode read.

GPIO 16 is free on ESP32-WROOM. On a WROVER board it is wired to PSRAM and cannot be used.

## Backend Integration

- **Relay status**: `smartcamper/sensors/module-5/status` → `applianceStatusUpdate`. Index 4 is a boiler mode active, not element current
- **Boiler**: `smartcamper/sensors/module-5/boiler` → `boilerStatusUpdate`
- **Urine level**: `smartcamper/sensors/toilet/urine/level` → `sensorUpdate` with `{ toiletUrineLevel }`
- **Force refresh**: `forceModuleUpdate` with `{ moduleId: "module-5" }` (relays, urine level, one electrode read)

## Frontend Integration

- **Boiler card**: click opens the boiler modal. The card lights while maintain or one-shot is active. Long press turns the boiler off. Long press while off does nothing
- **Boiler modal**: the top of the tank follows temperature (18°C and below dark blue, 35°C and above bright red). The bottom shifts from that color toward dark blue as it differs from the top, starting at 3°C and fully blue at 10°C. Half-full blinking blue water when `full` is false. Heating-element graphic only while `relay` is true. Power starts or stops maintain mode. 30° and 60° start a one-shot; pressing the lit one stops the boiler. + sits above the number and − below. They change the maintain setpoint only while maintain mode is on. The number updates immediately, and one command is sent 1.5 s after the last tap or after the button is released. Holding a button repeats one degree at a time. During a one-shot the number stays that setpoint. A preset button is inactive when the bottom sensor is already at that temperature. Power, 30°, and 60° need inverter or shore power to start, and loss of both stops an active mode
- **Toilet card** in main menu — `ToiletUrineTank`, yellow liquid, **WC** label
- **Detail modal** — polls module-5 every 5 s while open
- **Status panel** (tablet) — rotating slide after the toilet. No + or −. Power, 30°, and 60° show the active mode and do not send commands. The setpoint is shown only in maintain mode
- **Tablet display backlight** (tablet landscape only): long-press clock → off; tap overlay → on; Settings auto-off; wake when a light zone turns on (module-2). Relay index **6** in `applianceStatusUpdate`, no dedicated appliance card.

### Boiler ↔ 230 V (frontend rules)

230 V logic is in the frontend only (`frontend/src/constants/appliances.js`). The physical button can still enable the boiler without inverter or shore power. The ESP does block enable when the tank is not full, a temperature sensor has failed, or either sensor is at 75°C or above.

- **Enable turn-ON** when module-5 is online **and** at least one 230 V source is available:
  - inverter relay **ON**, or
  - live Victron **AC charger** BLE from module-6 (shore power; not stale / not `null`)
- **Always allow turn-OFF** when the boiler is already ON (even if both sources are gone)
- **Auto OFF** when both inverter and shore AC are absent (after a Victron snapshot if module-6 is online)
- Turning the **inverter OFF** auto-turns the boiler OFF **only if** shore AC is also absent
- Scenes that turn the inverter OFF follow the same rule (explicit `boilerOff` still forces OFF)

## Installation and Setup

1. **Upload firmware:**

   ```bash
   cd esp32-modules/module-5
   pio run --target upload
   ```

2. **Monitor serial output:**

   ```bash
   pio device monitor
   ```

   - Check WiFi/MQTT connection
   - Monitor button presses
   - Check relay status

3. **Test functionality:**
   - Test physical buttons
   - Check frontend dashboard
   - Test MQTT commands

## Troubleshooting

### Relays not working

- Check relay power supply (12V DC)
- Check relay connections (pins 14, 15, 23, 27, 32, 13 for display backlight; fridge power is on module-6 GPIO 14)
- Check serial output for errors
- Ensure proper ground connection between ESP32 and relays

### Buttons not working

- Check button connections (pins 17, 21, 25, 26, 33; fridge button is on module-6 GPIO 22)
- Check pull-up resistors (internal pull-ups are used)
- Test with multimeter (button should connect pin to GND when pressed)
- Check serial output for press messages

### No MQTT connection

- Check MQTT broker IP address in `src/Config.h`
- Check WiFi connection status
- Ensure MQTT broker is running on Raspberry Pi
- Check serial output for MQTT errors

### Urine level wrong or stuck

- Check electrodes GPIO 4 and 5, shared GND in tank
- Level is discrete (0 / 50 / 100) — not a continuous sensor
- Wait for mode window (~5 min) or send `force_update` for immediate reading

## Offline Operation

Module works independently:

- Physical buttons function without WiFi/MQTT
- Relays can be controlled via buttons offline
- Status updates are only published when connected
- No errors when offline - all local functions work

## Electrical Specifications

- **Relays**: Standard 12V relay module (NO/NC contacts)
- **Current consumption**: Depends on appliances (audio, pump, refrigerator)
- **Power supply**: 12V DC (sufficient capacity for all appliances)
- **Buttons**: Momentary push buttons (normally open)

## Architecture

- **ModuleManager**: WiFi, MQTT, Heartbeat, Commands
- **ApplianceManager**: Coordinates relays, buttons, urine sensor, commands
- **RelayController**: Manages relays (control, status)
- **ButtonHandler**: Processes button inputs (debouncing, state machine)
- **UrineLevelSensor**: Toilet tank level via conductivity electrodes

## Serial Debug Output

Enable/disable in `src/Config.h`:

- `DEBUG_SERIAL`: Serial console output
- `DEBUG_MQTT`: MQTT messages for publish/subscribe
- `DEBUG_VERBOSE`: Detailed relay state changes
