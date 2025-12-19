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

// MQTT topics
#define MQTT_TOPIC_PREFIX "smartcamper/"
#define MQTT_TOPIC_SENSORS "smartcamper/sensors/"
#define MQTT_TOPIC_COMMANDS "smartcamper/commands/"

// Timing settings
#define SENSOR_READ_INTERVAL 1000  // 1 second
#define HEARTBEAT_INTERVAL 10000   // 10 seconds - guaranteed send
#define MQTT_RECONNECT_DELAY 2000  // 2 seconds (reduced from 5s)
#define WIFI_RECONNECT_DELAY 3000  // 3 seconds (reduced from 10s)
#define WIFI_CHECK_INTERVAL 2000   // 2 seconds - WiFi connection check
#define WIFI_PING_TIMEOUT 1000     // 1 second timeout for ping

// Sensor threshold settings
#define TEMP_THRESHOLD 0.1     // 0.1°C change
#define HUMIDITY_THRESHOLD 1.0 // 1% change

// Debug settings
#define DEBUG_SERIAL true
#define DEBUG_MQTT true

#endif




