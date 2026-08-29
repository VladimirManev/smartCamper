# Ducato 2015 B-CAN signals (OBD pins 1+9, 50 kbit/s)

Discovered with ESP32 + WCMCU-230 listen-only sniffer. Keep for future module integration.

## Doors / handbrake — `E 06214000`

| Signal | Byte | Mask | Meaning |
|--------|------|------|---------|
| Handbrake | 0 | `0x20` | set = ON (applied) |
| Driver door | 1 | `0x04` | set = open |
| Passenger door | 1 | `0x08` | set = open |
| Sliding door | 1 | `0x30` | set = open |
| Rear door | 1 | `0x40` | set = open |

## Lights — `E 02214000`

| Signal | Byte | Mask | Notes |
|--------|------|------|-------|
| Parking lights | 1 | `0x60` | set = ON |
| High beam / flash | 1 | `0x10` | often pulse |
| Right turn | 2 | `0x20` | blinks |
| Left turn | 2 | `0x40` | blinks |
| Hazards | 2 | `0x60` | both turn bits |

## Locks — `E 02294000` (pulses, then often 00)

| Signal | Byte | Mask |
|--------|------|------|
| Lock both zones | 5 | `0x80` |
| Unlock front (cockpit) | 5 | `0x08` |
| Unlock rear | 6 | `0x80` |

## Reverse — `E 04214001`

| Signal | Byte | Mask |
|--------|------|------|
| Reverse gear | 7 | `0x04` | set = ON |

## Radio — `E 0E094024`

| State | Byte1 |
|-------|-------|
| ON | `0x1A` |
| OFF | `0x1E` |

## Hardware notes

- C-CAN (OBD 6+14, 500 kbit/s): engine/diag; sleeps without ignition; only partial door mirror.
- B-CAN (OBD 1+9, 50 kbit/s): body; remove WCMCU-230 120Ω terminator when tapping vehicle bus.
- ESP32 TWAI: TX GPIO17, RX GPIO16 (free on module-8).
