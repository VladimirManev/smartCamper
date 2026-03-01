#!/usr/bin/env python3
"""
Shutdown Button Service for Raspberry Pi
Hardware button with 2-second hold to trigger shutdown
LED indicator shows system is running (GPIO controlled)
"""

import RPi.GPIO as GPIO
import time
import subprocess
import sys
import signal
import logging

# Configuration
BUTTON_PIN = 3  # GPIO 3 (physical pin 5) - has built-in pull-up
LED_PIN = 4     # GPIO 4 (physical pin 7)
SHUTDOWN_HOLD_TIME = 2.0  # Seconds to hold button

# Logging setup
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('/var/log/shutdown-button.log'),
        logging.StreamHandler(sys.stdout)
    ]
)

logger = logging.getLogger(__name__)

# Global state
running = True
button_pressed_time = None

def signal_handler(sig, frame):
    """Handle shutdown signals"""
    global running
    logger.info("Received shutdown signal, stopping...")
    running = False
    cleanup()

def cleanup():
    """Cleanup GPIO pins"""
    logger.info("Cleaning up GPIO...")
    GPIO.output(LED_PIN, GPIO.LOW)  # Turn off LED
    GPIO.cleanup()
    logger.info("GPIO cleaned up")

def button_callback(channel):
    """Button interrupt callback"""
    global button_pressed_time
    
    if GPIO.input(BUTTON_PIN) == GPIO.LOW:
        # Button pressed
        button_pressed_time = time.time()
        logger.info("Button pressed")
    else:
        # Button released
        if button_pressed_time is not None:
            hold_duration = time.time() - button_pressed_time
            logger.info(f"Button released (held for {hold_duration:.2f}s)")
            
            if hold_duration >= SHUTDOWN_HOLD_TIME:
                logger.warning(f"Button held for {hold_duration:.2f}s - executing shutdown!")
                shutdown_pi()
            else:
                logger.info(f"Held for {hold_duration:.2f}s - insufficient for shutdown (need {SHUTDOWN_HOLD_TIME}s)")
        
        button_pressed_time = None

def shutdown_pi():
    """Execute shutdown command"""
    global running
    
    logger.warning("Starting shutdown process...")
    
    # Turn off LED before shutdown
    GPIO.output(LED_PIN, GPIO.LOW)
    time.sleep(0.1)
    
    # Execute shutdown
    try:
        subprocess.run(['sudo', 'shutdown', '-h', 'now'], check=True)
    except subprocess.CalledProcessError as e:
        logger.error(f"Error executing shutdown: {e}")
    except Exception as e:
        logger.error(f"Unexpected error: {e}")
    
    running = False

def main():
    """Main function"""
    global running, button_pressed_time
    
    logger.info("=" * 50)
    logger.info("Starting Shutdown Button Service")
    logger.info("=" * 50)
    logger.info(f"Button pin: GPIO {BUTTON_PIN}")
    logger.info(f"LED pin: GPIO {LED_PIN}")
    logger.info(f"Hold time: {SHUTDOWN_HOLD_TIME} seconds")
    logger.info("=" * 50)
    
    # Register signal handlers
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    
    # Setup GPIO
    try:
        GPIO.setmode(GPIO.BCM)
        GPIO.setwarnings(False)
        
        # Setup button pin (INPUT with pull-up)
        GPIO.setup(BUTTON_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)
        
        # Setup LED pin (OUTPUT)
        GPIO.setup(LED_PIN, GPIO.OUT)
        GPIO.output(LED_PIN, GPIO.LOW)  # Initially off
        
        # Register interrupt for button (both press and release)
        GPIO.add_event_detect(BUTTON_PIN, GPIO.BOTH, callback=button_callback, bouncetime=50)
        
        logger.info("GPIO initialized successfully")
        
        # Turn on LED to indicate service is running
        GPIO.output(LED_PIN, GPIO.HIGH)
        logger.info("LED ON - system is active")
        
        # Main loop
        logger.info("Waiting for button press...")
        
        while running:
            time.sleep(0.1)
            
            # Check for long hold (fallback mechanism)
            if button_pressed_time is not None:
                hold_duration = time.time() - button_pressed_time
                if hold_duration >= SHUTDOWN_HOLD_TIME:
                    # Check if button is still pressed
                    if GPIO.input(BUTTON_PIN) == GPIO.LOW:
                        logger.warning(f"Button held for {hold_duration:.2f}s - executing shutdown!")
                        shutdown_pi()
                        break
        
        logger.info("Service stopping")
        
    except KeyboardInterrupt:
        logger.info("Received KeyboardInterrupt")
    except Exception as e:
        logger.error(f"Error: {e}", exc_info=True)
    finally:
        cleanup()

if __name__ == "__main__":
    main()
