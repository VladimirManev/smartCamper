# LED Controller Module Documentation

## Overview

The LED Controller module is an ESP32-based multi-strip LED controller that manages up to 4 independent WS2815 RGBW LED strips. It supports button control, motion sensor activation, dimming, transitions, and synchronized control for kitchen lighting.

## Hardware Specifications

- **Microcontroller**: ESP32
- **LED Type**: WS2815 RGBW (12V)
- **Number of Strips**: 4
- **RMT Channels**: Each strip uses a separate RMT channel (RMT0-RMT3)

## LED Strips Configuration

| Strip Index | Pin | LED Count | RMT Channel | Description                    | Control                        |
| ----------- | --- | --------- | ----------- | ------------------------------ | ------------------------------ |
| **Strip 0** | 33  | 44        | RMT0        | Kitchen (main)                 | Button 1 (Pin 4)               |
| **Strip 1** | 18  | 178       | RMT1        | Main lighting                  | Button 2 (Pin 12)              |
| **Strip 2** | 19  | 23        | RMT2        | Kitchen extension (spice rack) | Auto-synced with Strip 0       |
| **Strip 3** | 25  | 53        | RMT3        | Bathroom                       | PIR sensor (Pin 2) - automatic |

### Strip Synchronization

- **Strip 0 and Strip 2** are synchronized in software
- When Strip 0 is turned on/off, Strip 2 automatically follows
- Both strips use the same brightness, transitions, and dimming
- They use different RMT channels but are controlled together

## Buttons Configuration

| Button       | Pin | Controls          | Description                                          |
| ------------ | --- | ----------------- | ---------------------------------------------------- |
| **Button 1** | 4   | Strip 0 + Strip 2 | Kitchen - toggles main and extension strips together |
| **Button 2** | 12  | Strip 1           | Main lighting control                                |
| **Button 3** | 27  | Floor lighting    | Floor lighting relay control (toggle only)           |

### Button Functions

- **Click**: Toggle strip ON/OFF (with random transitions)
- **Hold** (>250ms): Start dimming (increase/decrease brightness) - *Not available for Button 3*
- **Button 3**: Simple toggle for floor lighting relay (no dimming)

## Additional Circuits

### Floor Lighting (Relay Circuit)

| Component        | Pin | Direction | Description                    |
| ---------------- | --- | --------- | ------------------------------ |
| **Relay**        | 26  | Output    | Floor lighting relay control   |
| **Button 3**     | 27  | Input     | Floor lighting toggle button   |

**Floor Lighting Features:**
- Controlled via relay (GPIO 26)
- Toggle button on GPIO 27
- Simple ON/OFF control (no dimming)
- Uses standard LED diodes (not LED strips)
- Independent from LED strip controls

## Sensors Configuration

| Sensor                           | Pin | Controls           | Settings                       |
| -------------------------------- | --- | ------------------ | ------------------------------ |
| **PIR Motion Sensor (HC-SR501)** | 2   | Strip 3 (Bathroom) | Timeout: 60 seconds (1 minute) |

### Motion Sensor Behavior

- Automatically turns on Strip 3 when motion is detected
- Resets timeout timer on each motion detection
- Automatically turns off after 60 seconds of no motion
- No button control or dimming available for Strip 3

## Color Configuration

- **Color Mode**: White using RGB+W channels
- All channels (R, G, B, W) are set to the same brightness value
- Provides full-spectrum white light

## Brightness Settings

- **Minimum Brightness**: 1
- **Maximum Brightness**: 255
- **Default Brightness**: 128 (50% on first power on)
- **Dimming Speed**: 50 brightness units per second

## Features

### Transitions

The controller supports 8 different transition effects:

**Turn On Transitions:**

1. Center to Edges - Smoothly expands from center
2. Random LEDs - Random LEDs turn on sequentially
3. Left to Right - Sequential activation from left
4. Edges to Center - Expands from edges to center

**Turn Off Transitions:**

1. Edges to Center - Turns off from edges
2. Random LEDs - Random LEDs turn off sequentially
3. Left to Right - Sequential deactivation from left
4. Center to Edges - Turns off from center outward

- Transition duration: 1 second
- Randomly selected for each on/off action

### Dimming

- Activated by holding button for >250ms
- Smooth brightness increase/decrease
- Alternates between increasing and decreasing
- Blinks when reaching maximum brightness
- No dimming for Strip 3 (motion activated)

### Blinking

- Occurs when brightness reaches maximum
- Duration: 300ms
- Minimum brightness during blink: 30% of current brightness
- Visual feedback for maximum brightness

## Pin Summary

| Component      | Pin | Direction       | Description                 |
| -------------- | --- | --------------- | --------------------------- |
| **Strip 0**    | 33  | Output          | Kitchen main LED strip      |
| **Strip 1**    | 18  | Output          | Main lighting LED strip     |
| **Strip 2**    | 19  | Output          | Kitchen extension LED strip |
| **Strip 3**    | 25  | Output          | Bathroom LED strip          |
| **Button 1**   | 4   | Input (Pull-up) | Kitchen control button      |
| **Button 2**   | 12  | Input (Pull-up) | Main lighting button        |
| **Button 3**   | 27  | Input (Pull-up) | Floor lighting button       |
| **Relay**      | 26  | Output          | Floor lighting relay        |
| **PIR Sensor** | 2   | Input           | Motion detection sensor     |

## Technical Details

### RMT Channels

Each LED strip requires a dedicated RMT (Remote Control) channel on ESP32:

- Strip 0: RMT0
- Strip 1: RMT1
- Strip 2: RMT2
- Strip 3: RMT3

This allows independent timing control for each strip.

### Debouncing

- Button debounce delay: 50ms
- Separate debounce state machine for each button

### State Management

Each strip maintains:

- On/Off state
- Current brightness level
- Dimming state and direction
- Transition state
- Blink state

## Usage Examples

### Turning On Kitchen Lights

1. Press Button 1 (Pin 4)
2. Strip 0 turns on with random transition
3. Strip 2 automatically turns on simultaneously
4. Both strips maintain synchronized brightness

### Dimming Main Lighting

1. Press and hold Button 2 (Pin 12)
2. Brightness starts changing (increase/decrease alternates)
3. Release button to stop dimming
4. Brightness stays at current level

### Bathroom Motion Activation

1. Motion detected by PIR sensor (Pin 2)
2. Strip 3 automatically turns on
3. Timer resets on each motion detection
4. Automatically turns off after 60 seconds of no motion

### Floor Lighting Control

1. Press Button 3 (Pin 27)
2. Relay (Pin 26) toggles ON/OFF
3. Controls floor lighting circuit with standard LED diodes
4. Simple toggle - no dimming or transitions

## Notes

- Strip 2 (Kitchen extension) cannot be controlled independently
- Strip 3 (Bathroom) has no button control or dimming
- All strips use white color with RGB+W channels
- Transitions are randomly selected for visual variety
- System initializes all strips to OFF state
- Floor lighting (relay circuit) is independent from LED strips
- Floor lighting uses standard LED diodes, not LED strips
- Floor lighting has simple toggle control (no dimming)

## Version Information

- Module: LED Controller
- Last Updated: 2024
- Hardware: ESP32
- LED Type: WS2815 RGBW
