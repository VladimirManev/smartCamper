// ESP32 Common Configuration
// Common configuration for LED Controller module

#ifndef CONFIG_H
#define CONFIG_H

// WiFi settings
#define WIFI_SSID "SmartCamper"
#define WIFI_PASSWORD "12344321"

// MQTT settings - CHANGE IP OF COMPUTER WHERE BACKEND RUNS!
#define MQTT_BROKER_IP "192.168.4.1"  // Pi IP in SmartCamper network
#define MQTT_BROKER_PORT 1883
#define MQTT_CLIENT_ID_PREFIX "smartcamper_led_"

// MQTT topics
#define MQTT_TOPIC_PREFIX "smartcamper/"
#define MQTT_TOPIC_SENSORS "smartcamper/sensors/"
#define MQTT_TOPIC_COMMANDS "smartcamper/commands/"

// Hardware settings
#define NUM_RELAYS 1     // Number of relays (easily expandable)

// Timing settings
#define MQTT_RECONNECT_DELAY 2000  // 2 seconds
#define WIFI_RECONNECT_DELAY 3000  // 3 seconds
#define WIFI_CHECK_INTERVAL 2000   // 2 seconds - WiFi connection check
#define WIFI_PING_TIMEOUT 1000     // 1 second timeout for ping
#define STATUS_PUBLISH_INTERVAL 5000  // 5 seconds - status publish (deprecated, not used)
#define HEARTBEAT_INTERVAL 10000  // 10 seconds - interval for sending full status

// Debug settings
#define DEBUG_SERIAL true
#define DEBUG_MQTT true
#define DEBUG_VERBOSE false  // Verbose messages (Kitchen sync, details)

#endif

