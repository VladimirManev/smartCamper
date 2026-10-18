# AUTOTERM Air 2D — stage 2 transparent bridge

ESP32 is a serial proxy between Comfort Control and the Air 2D ECU. Bytes are forwarded unchanged both ways and logged on USB.

Fuel pump, fan, glow plug, ignition, shutdown, and temperature limits stay in the original ECU. This firmware does not inject commands.

Confirmed on this pair: **9600 8N1**, frames `AA 03` (panel) / `AA 04` (heater), CRC16 Modbus.

Bulgarian notes: `README_BG.md`.

## Wiring

Direct panel↔heater TX/RX wires are cut. ESP32 reconnects them through two ADuM1201 modules:

```
Comfort Control TX ---- ADuM#1 VIA (7) ---- VOA (2) ---- ESP32 GPIO 34 RX
Comfort Control RX ---- ADuM#1 VOB (6) ---- VIB (3) ---- ESP32 GPIO 27 TX

Air 2D TX ------------- ADuM#2 VIA (7) ---- VOA (2) ---- ESP32 GPIO 32 RX
Air 2D RX ------------- ADuM#2 VOB (6) ---- VIB (3) ---- ESP32 GPIO 33 TX
```

| Port | UART | RX | TX |
|------|------|----|----|
| PANEL | UART1 | GPIO 34 | GPIO 27 |
| HEATER | UART2 | GPIO 32 | GPIO 33 |

## Build

```bash
cd esp32-modules/autoterm-sniffer
pio run -t upload
pio device monitor
```

Success = Comfort Control behaves normally and the log shows paired `PANEL -> HEATER` / `HEATER -> PANEL` frames.

## Next

Command injection is not in this firmware yet.
