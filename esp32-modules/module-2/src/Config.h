// ESP32 Common Configuration
// Common configuration for all ESP32 modules

#ifndef CONFIG_H
#define CONFIG_H

// WiFi settings
#define WIFI_SSID "SmartCamper"
#define WIFI_PASSWORD "12344321"

// MQTT settings - CHANGE IP OF COMPUTER WHERE BACKEND RUNS!
#define MQTT_BROKER_IP "192.168.4.1"  // Pi IP in SmartCamper network
#define MQTT_BROKER_PORT 1883
#define MQTT_CLIENT_ID_PREFIX "smartcamper_"

// Module identification
#define MODULE_ID "module-2"  // Unique identifier for this ESP32 module

// MQTT topics
#define MQTT_TOPIC_PREFIX "smartcamper/"
#define MQTT_TOPIC_SENSORS "smartcamper/sensors/"
#define MQTT_TOPIC_COMMANDS "smartcamper/commands/"

// Timing settings
#define HEARTBEAT_INTERVAL 10000   // 10 seconds - guaranteed send
#define MQTT_RECONNECT_DELAY 2000  // 2 seconds (reduced from 5s)
#define WIFI_RECONNECT_DELAY 3000  // 3 seconds (reduced from 10s)
#define WIFI_CHECK_INTERVAL 2000   // 2 seconds - WiFi connection check
#define WIFI_PING_TIMEOUT 1000     // 1 second timeout for ping

// LED Strip Configuration (Module 2 specific)
#define NUM_STRIPS 5     // Number of LED strips

// Strip settings - pins and LED counts
#define STRIP_0_PIN 33
#define STRIP_0_LED_COUNT 44    // Kitchen (main)
#define STRIP_1_PIN 18
#define STRIP_1_LED_COUNT 178   // Main lighting
#define STRIP_2_PIN 19
#define STRIP_2_LED_COUNT 23    // Kitchen (extension for spice rack, mirrors strip 0)
#define STRIP_3_PIN 25
#define STRIP_3_LED_COUNT 53    // Bathroom (motion activated, no button, no dimming)
#define STRIP_4_PIN 5
#define STRIP_4_LED_COUNT 30    // Bedroom (GRBW protocol)

// Button settings
#define NUM_BUTTONS 4    // Number of buttons (Strip 2 is automatically controlled by Strip 0)
#define BUTTON_PIN_1 4   // Button for strip 0 (Kitchen - controls Strip 0 and Strip 2)
#define BUTTON_PIN_2 12  // Button for strip 1
#define BUTTON_PIN_3 27  // Button for relay circuit (toggle button)
#define BUTTON_PIN_4 13  // Button for strip 4 (Bedroom)

// Relay settings (for LED diodes circuit)
#define NUM_RELAYS 1     // Number of relays (easily expandable)
#define RELAY_PIN_0 26   // Pin for relay 0 control (OUTPUT)

// PIR sensor settings (HC-SR501)
#define PIR_SENSOR_PIN 2        // Pin for PIR sensor
#define PIR_MOTION_TIMEOUT 60000  // 60 seconds (1 minute) after last motion detected
#define MOTION_STRIP_INDEX 3    // Strip 3 (Bathroom) is controlled by the sensor

// Brightness settings
#define MIN_BRIGHTNESS 1
#define MAX_BRIGHTNESS 255
#define DEFAULT_BRIGHTNESS 128  // 50% on first power on

// Dimming settings
#define DIMMING_SPEED 50  // brightness units per second (change speed)
#define HOLD_THRESHOLD 250  // 250ms before dimming starts
#define BLINK_DURATION 300  // Blink duration at max brightness (ms)
#define BLINK_MIN_FACTOR 0.3  // Minimum brightness during blink (30% of current)

// Transition settings
#define TRANSITION_DURATION 1000  // 1 second for transitions
#define NUM_ON_TRANSITIONS 4   // Number of transitions for turning on
#define NUM_OFF_TRANSITIONS 4  // Number of transitions for turning off

// Debug settings
#define DEBUG_SERIAL true   // Enable serial debug output
#define DEBUG_MQTT true     // Enable MQTT debug output
#define DEBUG_VERBOSE false  // Enable verbose debug output (set to true for detailed logging)

// Watchdog settings (optional - for production stability)
// #define ENABLE_WATCHDOG true
// #define WATCHDOG_TIMEOUT 30  // seconds

#endif

