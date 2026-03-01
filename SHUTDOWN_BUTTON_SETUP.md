# Shutdown Button Setup for Raspberry Pi

## Description

Hardware button for safe shutdown of Raspberry Pi. Holding the button for 2 seconds executes shutdown command. LED indicator shows that the system is running.

## Hardware Configuration

### Required Components

- 1x Tactile push button (momentary)
- 1x LED diode
- 1x Resistor 220-330Ω (for LED)
- Jumper wires

### Wiring Diagram

#### Button
- **One end** → GPIO 3 (physical pin 5)
- **Other end** → GND (physical pin 6, 9, 14, 20, 25, 30, 34, or 39)

**Note:** GPIO 3 has built-in pull-up resistor, no external resistor needed.

#### LED Indicator
- **Anode (long leg)** → GPIO 4 (physical pin 7) through 220-330Ω resistor
- **Cathode (short leg)** → GND

**Important:** LED must have a current-limiting resistor! Without resistor, the GPIO pin may be damaged.

### Physical Pins

```
    3.3V  [1]  [2]  5V
   GPIO2  [3]  [4]  5V
   GPIO3  [5]  [6]  GND  ← Button here
   GPIO4  [7]  [8]  GPIO14  ← LED here
     GND  [9]  [10] GPIO15
```

**Used pins:**
- **GPIO 3 (physical pin 5)** - Button
- **GPIO 4 (physical pin 7)** - LED

## Software Installation

### Step 1: Upload Files

Files are uploaded automatically with `deploy-to-raspberry.sh` or manually:
- `shutdown-button.py` → `~/smartCamper/shutdown-button.py`
- `shutdown-button.service` → `~/smartCamper/shutdown-button.service`
- `install-shutdown-button.sh` → `~/smartCamper/install-shutdown-button.sh`

### Step 2: Installation

```bash
cd ~/smartCamper
chmod +x install-shutdown-button.sh
./install-shutdown-button.sh
```

The script automatically:
- Installs RPi.GPIO (if missing)
- Sets permissions
- Installs systemd service
- Starts service

### Step 3: Verification

```bash
# Status
sudo systemctl status shutdown-button.service

# Logs
sudo journalctl -u shutdown-button.service -f
```

## How It Works

1. **LED Indicator:**
   - LED lights up when Pi is on and service is running
   - LED turns off before shutdown
   - LED does not light when Pi is off (GPIO has no power)

2. **Shutdown Button:**
   - Press for < 2 seconds - no action
   - Hold for 2+ seconds - executes `sudo shutdown -h now`

3. **Safety:**
   - Protection against accidental presses (2 second hold)
   - Logging of all actions

## Troubleshooting

### LED lights even when Pi is shutdown

**Cause:** LED is connected directly to 3.3V/5V instead of GPIO pin.

**Solution:** Connect LED to GPIO 4 (physical pin 7) with current-limiting resistor. GPIO pins turn off during shutdown.

### Button doesn't work

**Checks:**
1. Verify button wiring
2. Check status: `sudo systemctl status shutdown-button.service`
3. Check logs: `sudo journalctl -u shutdown-button.service -n 50`

### Service doesn't start

**Checks:**
1. Verify path in service file
2. Check if script is executable: `ls -l ~/smartCamper/shutdown-button.py`
3. Check if RPi.GPIO is installed: `pip3 list | grep RPi.GPIO`

## Configuration Changes

To change GPIO pins or hold time, edit `shutdown-button.py`:

```python
BUTTON_PIN = 3  # Change GPIO pin for button
LED_PIN = 4     # Change GPIO pin for LED
SHUTDOWN_HOLD_TIME = 2.0  # Change time in seconds
```

After changes:
```bash
sudo systemctl restart shutdown-button.service
```

## Removal

```bash
# Stop
sudo systemctl stop shutdown-button.service

# Disable
sudo systemctl disable shutdown-button.service

# Remove
sudo rm /etc/systemd/system/shutdown-button.service
sudo systemctl daemon-reload
```
