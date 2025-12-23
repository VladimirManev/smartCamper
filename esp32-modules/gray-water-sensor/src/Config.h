// ESP32 Gray Water Sensor Configuration
// Configuration for gray water level sensor module

#ifndef CONFIG_H
#define CONFIG_H

// WiFi settings
#define WIFI_SSID "SmartCamper"
#define WIFI_PASSWORD "12344321"

// MQTT settings - CHANGE IP OF COMPUTER WHERE BACKEND RUNS!
#define MQTT_BROKER_IP "192.168.4.1"  // Pi IP in SmartCamper network
#define MQTT_BROKER_PORT 1883
#define MQTT_CLIENT_ID_PREFIX "smartcamper_"

// MQTT topics
#define MQTT_TOPIC_PREFIX "smartcamper/"
#define MQTT_TOPIC_SENSORS "smartcamper/sensors/"
#define MQTT_TOPIC_COMMANDS "smartcamper/commands/"

// Timing settings
#define SENSOR_READ_INTERVAL 1000      // 1 second - measurement interval
#define AVERAGE_INTERVAL 5000          // 5 seconds - average calculation interval
#define HEARTBEAT_INTERVAL 15000       // 15 seconds - guaranteed publish
#define MQTT_RECONNECT_DELAY 2000      // 2 seconds
#define WIFI_RECONNECT_DELAY 3000       // 3 seconds
#define WIFI_CHECK_INTERVAL 2000       // 2 seconds - WiFi connection check
#define WIFI_PING_TIMEOUT 1000         // 1 second timeout for ping

// GPIO Pin Configuration (from bottom to top)
// Pin 1 (bottom) = 15%, Pin 7 (top) = 100%
#define WATER_LEVEL_PIN_1 4   // GPIO 4 - 15% level
#define WATER_LEVEL_PIN_2 5    // GPIO 5 - 30% level
#define WATER_LEVEL_PIN_3 18   // GPIO 18 - 45% level
#define WATER_LEVEL_PIN_4 19   // GPIO 19 - 60% level
#define WATER_LEVEL_PIN_5 21   // GPIO 21 - 75% level
#define WATER_LEVEL_PIN_6 22   // GPIO 22 - 90% level
#define WATER_LEVEL_PIN_7 23   // GPIO 23 - 100% level
#define GND_BOLT_PIN -1        // GND (not a GPIO pin, connected to ESP32 GND)

// Number of level pins
#define NUM_LEVEL_PINS 7

// Level percentages (from bottom to top)
#define LEVEL_PERCENT_1 15
#define LEVEL_PERCENT_2 30
#define LEVEL_PERCENT_3 45
#define LEVEL_PERCENT_4 60
#define LEVEL_PERCENT_5 75
#define LEVEL_PERCENT_6 90
#define LEVEL_PERCENT_7 100

// Debug settings
#define DEBUG_SERIAL true
#define DEBUG_MQTT true

#endif

