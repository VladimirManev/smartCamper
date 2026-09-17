# Future ideas

Ideas for later — not scheduled. Keep entries short; expand when starting work.

---

## Wireless BLE perimeter motion sensors

**Goal:** Battery PIR sensors ~10–15 m from the camper that feed **Zone 2 (perimeter)** on module-8.

**Hardware (on hand):** Seeed XIAO nRF52840 + SR312 PIR.

**Idea:** On motion, the nRF briefly **BLE-advertises** a known ID (no pairing, no connect, no payload beyond “I’m here”). Module-8 only **scans** while Zone 2 is armed; seeing a known beacon = same as a wired perimeter PIR (buzzer alert).

**Notes:**
- Advertise only on motion, then deep sleep (battery).
- ESP32 WiFi + BLE share the radio — use short scan bursts (like module-6 Victron), not continuous scan.
- Whitelist by MAC / manufacturer data so random BLE devices do not trip.

**Status:** Idea only — implement later.
