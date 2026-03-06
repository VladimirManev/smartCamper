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
#define MODULE_ID "module-5" // Unique identifier for this ESP32 module

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

// Appliance Relay Configuration (Module 5 specific)
#define NUM_RELAYS 5 // Number of appliance relays

// Relay pins (for controlling appliances)
#define RELAY_PIN_0 14 // GPIO pin for relay 0 (Audio System)
#define RELAY_PIN_1 15 // GPIO pin for relay 1 (Water Pump)
#define RELAY_PIN_2 16 // GPIO pin for relay 2 (Refrigerator)
#define RELAY_PIN_3 23 // GPIO pin for relay 3 (WC Fan)
#define RELAY_PIN_4 27 // GPIO pin for relay 4 (Boiler)

// Button pins (for manual control - toggle buttons)
#define BUTTON_PIN_0 17 // GPIO pin for button 0 (Audio System)
#define BUTTON_PIN_1 21 // GPIO pin for button 1 (Water Pump)
#define BUTTON_PIN_2 22 // GPIO pin for button 2 (Refrigerator)
#define BUTTON_PIN_3 25 // GPIO pin for button 3 (WC Fan)
#define BUTTON_PIN_4 26 // GPIO pin for button 4 (Boiler)

#define NUM_BUTTONS 5 // Number of buttons

// Appliance names (for identification)
// Relay 0: Audio System
// Relay 1: Water Pump
// Relay 2: Refrigerator
// Relay 3: WC Fan
// Relay 4: Boiler

// Debug settings
#define DEBUG_SERIAL true   // Enable serial debug output
#define DEBUG_MQTT true     // Enable MQTT debug output
#define DEBUG_VERBOSE false // Enable verbose debug output (set to true for detailed logging)

// Watchdog settings (optional - for production stability)
// #define ENABLE_WATCHDOG true
// #define WATCHDOG_TIMEOUT 30  // seconds

#endif
