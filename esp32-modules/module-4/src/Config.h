// ESP32 Common Configuration
// Common configuration for all ESP32 modules

#ifndef CONFIG_H
#define CONFIG_H

// WiFi settings
#define WIFI_SSID "SmartCamper"
#define WIFI_PASSWORD "12344321"

// MQTT settings - CHANGE IP OF COMPUTER WHERE BACKEND RUNS!
#define MQTT_BROKER_IP "192.168.4.1" // Pi IP in SmartCamper network
#define MQTT_BROKER_PORT 1883
#define MQTT_CLIENT_ID_PREFIX "smartcamper_"

// Module identification
#define MODULE_ID "module-4" // Unique identifier for this ESP32 module

// MQTT topics
#define MQTT_TOPIC_PREFIX "smartcamper/"
#define MQTT_TOPIC_SENSORS "smartcamper/sensors/"
#define MQTT_TOPIC_COMMANDS "smartcamper/commands/"

// Timing settings
#define HEARTBEAT_INTERVAL 10000  // 10 seconds - guaranteed send
#define MQTT_RECONNECT_DELAY 2000 // 2 seconds (reduced from 5s)
#define WIFI_RECONNECT_DELAY 3000 // 3 seconds (reduced from 10s)
#define WIFI_CHECK_INTERVAL 2000  // 2 seconds - WiFi connection check
#define WIFI_PING_TIMEOUT 1000    // 1 second timeout for ping

// Module 4 specific configuration - Heating Control
// Damper (air vent) configuration
#define NUM_DAMPERS 5 // Number of dampers

// Servo motor pins for each damper (array indices match damper indices)
#define DAMPER_0_SERVO_PIN 2  // GPIO pin for damper 0 servo signal (Yellow/Orange wire)
#define DAMPER_0_BUTTON_PIN 4 // GPIO pin for damper 0 toggle button
#define DAMPER_1_SERVO_PIN 5  // GPIO pin for damper 1 servo signal
#define DAMPER_1_BUTTON_PIN 16 // GPIO pin for damper 1 toggle button
#define DAMPER_2_SERVO_PIN 12 // GPIO pin for damper 2 servo signal
#define DAMPER_2_BUTTON_PIN 17 // GPIO pin for damper 2 toggle button
#define DAMPER_3_SERVO_PIN 13 // GPIO pin for damper 3 servo signal
#define DAMPER_3_BUTTON_PIN 18 // GPIO pin for damper 3 toggle button
#define DAMPER_4_SERVO_PIN 14 // GPIO pin for damper 4 servo signal
#define DAMPER_4_BUTTON_PIN 19 // GPIO pin for damper 4 toggle button

// Table (lift) configuration
#define TABLE_RELAY_UP_PIN 21   // GPIO pin for table relay up (NO contact -> +)
#define TABLE_RELAY_DOWN_PIN 22 // GPIO pin for table relay down (NO contact -> +)
#define TABLE_BUTTON_UP_PIN 23  // GPIO pin for table button up (INPUT_PULLUP)
#define TABLE_BUTTON_DOWN_PIN 25 // GPIO pin for table button down (INPUT_PULLUP)

// Table timing settings
#define TABLE_DOUBLE_CLICK_TIMEOUT 500  // ms - max time between clicks for double-click detection
#define TABLE_AUTO_MOVE_DURATION 20000  // ms - duration for auto movement on double-click
#define TABLE_START_DELAY 600           // ms - delay before starting motor (longer than double-click timeout)

// Debug settings
#define DEBUG_SERIAL true   // Enable serial debug output
#define DEBUG_MQTT true     // Enable MQTT debug output
#define DEBUG_VERBOSE false // Enable verbose debug output (set to true for detailed logging)

// Watchdog settings (optional - for production stability)
// #define ENABLE_WATCHDOG true
// #define WATCHDOG_TIMEOUT 30  // seconds

#endif
