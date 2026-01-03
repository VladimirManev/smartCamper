# Gray Water Level Sensor Module Documentation

## Overview

The Gray Water Level Sensor module is an ESP32-based sensor that monitors the water level in a gray water tank using 7 stainless steel bolts as electrodes. The module measures electrical conductivity between a common GND bolt and 7 level-detection bolts positioned at different heights. It publishes water level data via MQTT and supports remote force-update commands.

## Hardware Specifications

- **Microcontroller**: ESP32
- **Sensor Type**: Conductivity-based level detection
- **Electrodes**: 8 stainless steel bolts (1 GND + 7 level detection)
- **Number of Level Pins**: 7
- **Measurement Method**: Digital pin reading (LOW = covered, HIGH = not covered)

## Hardware Configuration

### Electrode Setup

The gray water tank has 8 stainless steel bolts mounted on one wall:

- **1 GND Bolt**: Connected to ESP32 GND (common reference)
- **7 Level Bolts**: Positioned at different heights, connected to GPIO pins

### Level Detection Bolts (from bottom to top)

| Bolt       | GPIO Pin | Level Percentage | Description                   |
| ---------- | -------- | ---------------- | ----------------------------- |
| **Bolt 1** | 4        | 15%              | Lowest level detection point  |
| **Bolt 2** | 5        | 30%              | Second level detection point  |
| **Bolt 3** | 18       | 45%              | Third level detection point   |
| **Bolt 4** | 19       | 60%              | Fourth level detection point  |
| **Bolt 5** | 21       | 75%              | Fifth level detection point   |
| **Bolt 6** | 22       | 90%              | Sixth level detection point   |
| **Bolt 7** | 23       | 100%             | Highest level detection point |

### Pin Configuration

| Component       | Pin | Direction       | Description                |
| --------------- | --- | --------------- | -------------------------- |
| **Level Pin 1** | 4   | Input (Pull-up) | 15% level detection        |
| **Level Pin 2** | 5   | Input (Pull-up) | 30% level detection        |
| **Level Pin 3** | 18  | Input (Pull-up) | 45% level detection        |
| **Level Pin 4** | 19  | Input (Pull-up) | 60% level detection        |
| **Level Pin 5** | 21  | Input (Pull-up) | 75% level detection        |
| **Level Pin 6** | 22  | Input (Pull-up) | 90% level detection        |
| **Level Pin 7** | 23  | Input (Pull-up) | 100% level detection       |
| **GND Bolt**    | GND | Ground          | Common reference electrode |

## Measurement Algorithm

### Level Detection Logic

1. **Measurement Process**:

   - Pins are configured as INPUT_PULLUP before measurement
   - Measurement scans from top to bottom (Pin 7 → Pin 1)
   - Stops at the first pin that reads LOW (covered by water)
   - After measurement, all pins are set to LOW (INPUT mode) to minimize current flow

2. **Why Top-to-Bottom?**:

   - If a higher bolt is covered, all lower bolts are also covered
   - Finding the highest covered bolt gives the current water level
   - More efficient than checking all pins

3. **Corrosion Prevention**:
   - Pins are set to LOW (INPUT mode) after each measurement
   - This minimizes current flow through water between measurements
   - Reduces electrode corrosion over time

### Data Processing

1. **Individual Measurements**:

   - Measurement taken every 1 second
   - Each measurement returns a level index (0-6) or -1 for 0%
   - Level index is converted to percentage (15%, 30%, 45%, 60%, 75%, 90%, 100%)

2. **Averaging**:

   - Last 5 measurements are stored in a buffer
   - Every 5 seconds, average of the 5 measurements is calculated
   - Average is rounded to 1 decimal place

3. **Publishing Logic**:
   - Publish if value changed (>0.1% threshold) OR
   - Publish if heartbeat interval (15 seconds) elapsed OR
   - Publish on first measurement
   - If no change, republish same value every 15 seconds (heartbeat)

## Communication Protocol

### MQTT Topics

**Status Publishing:**

- `smartcamper/sensors/gray-water/level` - Water level percentage (0-100%)

**Command Topics:**

- `smartcamper/commands/gray-water-sensor/force_update` - Force immediate measurement and publish

### MQTT Message Format

**Level Message:**

```
Topic: smartcamper/sensors/gray-water/level
Payload: "45.2"  (percentage as string, 1 decimal place)
```

**Example Values:**

- `"0.0"` - Tank is empty (no bolts covered)
- `"15.0"` - Water reached Bolt 1 (15% level)
- `"45.0"` - Water reached Bolt 3 (45% level)
- `"100.0"` - Tank is full (Bolt 7 covered)

### Technical Details

- MQTT buffer size: 128 bytes (default PubSubClient)
- Status published every 15 seconds (heartbeat interval)
- Status also published on any value change (>0.1% threshold)
- MQTT QoS: Level 0 (at most once delivery)

## Timing Configuration

| Setting                | Value    | Description                                     |
| ---------------------- | -------- | ----------------------------------------------- |
| **Read Interval**      | 1000 ms  | Time between individual measurements            |
| **Average Interval**   | 5000 ms  | Time between average calculations               |
| **Heartbeat Interval** | 15000 ms | Guaranteed publish interval (even if no change) |
| **MQTT Reconnect**     | 2000 ms  | Delay between MQTT reconnection attempts        |
| **WiFi Reconnect**     | 3000 ms  | Delay between WiFi reconnection attempts        |

## Features

### Automatic Level Detection

- Scans from top to bottom to find highest covered bolt
- Efficient algorithm stops at first covered bolt
- Handles edge cases (0%, 100%, intermediate levels)

### Data Smoothing

- 5-second averaging reduces noise from water movement
- Prevents rapid fluctuations in reported level
- Provides stable readings for dashboard display

### Corrosion Prevention

- Pins set to LOW after each measurement
- Minimizes current flow through water
- Extends electrode lifespan

### Heartbeat System

- Guaranteed publish every 15 seconds
- Allows backend to detect offline modules
- Same value republished if no change occurred

### Force Update Support

- Backend can request immediate measurement
- Useful for health checks and manual refresh
- Responds to MQTT force_update command

## Network Features

- **Non-blocking WiFi**: WiFi connection attempts do not block main loop
- **Auto-reconnect**: Automatic reconnection to WiFi and MQTT when connection is lost
- **Active Connection Check**: Verifies WiFi connection is actually working (not just status)
- **Local operation**: Module continues measuring even without WiFi/MQTT (data buffered)

## Usage Examples

### Normal Operation

1. Module starts and connects to WiFi
2. Every 1 second: measures water level
3. Every 5 seconds: calculates average of last 5 measurements
4. Publishes if value changed or heartbeat interval elapsed
5. Backend receives level updates via MQTT
6. Frontend displays current water level

### Force Update

1. Backend sends MQTT command: `smartcamper/commands/gray-water-sensor/force_update`
2. Module immediately performs measurement
3. Calculates average if 5 measurements available
4. Publishes current level (even if unchanged)
5. Useful for health checks and manual refresh

### Empty Tank Detection

1. All pins read HIGH (not covered)
2. `readWaterLevel()` returns -1
3. Converted to 0.0% in `levelToPercent()`
4. Published as `"0.0"` on MQTT topic

### Full Tank Detection

1. Top bolt (Pin 7, GPIO 23) reads LOW (covered)
2. `readWaterLevel()` returns 6 (highest level index)
3. Converted to 100.0% in `levelToPercent()`
4. Published as `"100.0"` on MQTT topic

## Technical Details

### Pin State Logic

- **HIGH (1)**: Pin not covered by water (no connection to GND)
- **LOW (0)**: Pin covered by water (connection to GND through water)

### Measurement Sequence

1. Configure all pins as INPUT_PULLUP
2. Wait 10ms for pull-up to stabilize
3. Read pins from top to bottom (Pin 7 → Pin 1)
4. Return index of first LOW pin (or -1 if all HIGH)
5. Set all pins to LOW (INPUT mode) to minimize current

### Level to Percentage Mapping

| Level Index | Bolt   | Percentage |
| ----------- | ------ | ---------- |
| -1          | None   | 0%         |
| 0           | Bolt 1 | 15%        |
| 1           | Bolt 2 | 30%        |
| 2           | Bolt 3 | 45%        |
| 3           | Bolt 4 | 60%        |
| 4           | Bolt 5 | 75%        |
| 5           | Bolt 6 | 90%        |
| 6           | Bolt 7 | 100%       |

### Data Flow

```
Measurement (1s) → Buffer (5 values) → Average (5s) → Publish (if changed or heartbeat)
```

## Notes

- Module uses same structure as temperature-sensor module
- All pins are set to LOW after measurement to prevent corrosion
- Measurement is non-blocking (doesn't delay other operations)
- Module continues measuring even if WiFi/MQTT disconnected
- Average calculation only happens after 5 measurements collected
- First publish happens after first average calculation (5 seconds)
- Heartbeat ensures backend always knows module is alive
- Force update allows backend to request immediate data
- GPIO pins chosen to avoid boot/flash conflicts
- All pins support INPUT_PULLUP mode

## Version Information

- Module: Gray Water Level Sensor
- Last Updated: 2025
- Hardware: ESP32
- Sensor Type: Conductivity-based (stainless steel bolts)
- Number of Level Pins: 7
- Measurement Interval: 1 second
- Average Interval: 5 seconds
- Heartbeat Interval: 15 seconds
