// ESP32 Common Configuration
// Обща конфигурация за всички ESP32 модули

#ifndef CONFIG_H
#define CONFIG_H

// WiFi настройки
#define WIFI_SSID "SmartCamper_WiFi"
#define WIFI_PASSWORD "smartcamper123"

// MQTT настройки
#define MQTT_BROKER_IP "192.168.1.100"  // IP на Raspberry Pi
#define MQTT_BROKER_PORT 1883
#define MQTT_CLIENT_ID_PREFIX "smartcamper_"

// MQTT топици
#define MQTT_TOPIC_PREFIX "smartcamper/"
#define MQTT_TOPIC_SENSORS "smartcamper/sensors/"
#define MQTT_TOPIC_COMMANDS "smartcamper/commands/"

// Timing настройки
#define SENSOR_READ_INTERVAL 5000    // 5 секунди
#define MQTT_RECONNECT_DELAY 5000    // 5 секунди
#define WIFI_RECONNECT_DELAY 10000   // 10 секунди

// Debug настройки
#define DEBUG_SERIAL true
#define DEBUG_MQTT true

#endif
