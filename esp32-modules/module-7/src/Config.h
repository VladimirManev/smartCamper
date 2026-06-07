// ESP32 Module 7 Configuration
// Clean water level sensor module

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
#define MODULE_ID "module-7"

// MQTT topics
#define MQTT_TOPIC_PREFIX "smartcamper/"
#define MQTT_TOPIC_SENSORS "smartcamper/sensors/"
#define MQTT_TOPIC_COMMANDS "smartcamper/commands/"

// Timing settings
#define HEARTBEAT_INTERVAL 10000  // 10 seconds
#define MQTT_RECONNECT_DELAY 2000 // 2 seconds
#define WIFI_RECONNECT_DELAY 3000 // 3 seconds
#define WIFI_CHECK_INTERVAL 2000  // 2 seconds
#define WIFI_PING_TIMEOUT 1000    // 1 second

// Clean Water Level Sensor Configuration
// GPIO pins for level detection (from bottom to top)
#define CLEAN_WATER_LEVEL_PIN_1 4   // GPIO 4 - 15% level
#define CLEAN_WATER_LEVEL_PIN_2 5   // GPIO 5 - 30% level
#define CLEAN_WATER_LEVEL_PIN_3 18  // GPIO 18 - 45% level
#define CLEAN_WATER_LEVEL_PIN_4 19  // GPIO 19 - 60% level
#define CLEAN_WATER_LEVEL_PIN_5 21  // GPIO 21 - 75% level
#define CLEAN_WATER_LEVEL_PIN_6 22  // GPIO 22 - 90% level
#define CLEAN_WATER_LEVEL_PIN_7 23  // GPIO 23 - 100% level
#define NUM_CLEAN_WATER_LEVEL_PINS 7

// Level percentages (from bottom to top)
#define CLEAN_WATER_LEVEL_PERCENT_1 15
#define CLEAN_WATER_LEVEL_PERCENT_2 30
#define CLEAN_WATER_LEVEL_PERCENT_3 45
#define CLEAN_WATER_LEVEL_PERCENT_4 60
#define CLEAN_WATER_LEVEL_PERCENT_5 75
#define CLEAN_WATER_LEVEL_PERCENT_6 90
#define CLEAN_WATER_LEVEL_PERCENT_7 100

// Clean water level sensor timing
#define CLEAN_WATER_LEVEL_READ_INTERVAL 30000   // 30 seconds - electrode-friendly
#define CLEAN_WATER_LEVEL_MODE_SAMPLE_COUNT 10  // rolling window: 10 x 30s ~= 5 min
#define CLEAN_WATER_LEVEL_THRESHOLD 1.0         // 1% change threshold for publishing

// Debug settings
#define DEBUG_SERIAL true
#define DEBUG_MQTT false
#define DEBUG_VERBOSE false

#endif
