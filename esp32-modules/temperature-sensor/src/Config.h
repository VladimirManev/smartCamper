// ESP32 Common Configuration
// Обща конфигурация за всички ESP32 модули

#ifndef CONFIG_H
#define CONFIG_H

// WiFi настройки
#define WIFI_SSID "SmartCamper"
#define WIFI_PASSWORD "12344321"

// MQTT настройки - ПРОМЕНИ НА IP НА КОМПЮТЪРА КЪДЕТО РАБОТИ BACKEND!
#define MQTT_BROKER_IP "192.168.4.1"  // Pi IP в SmartCamper мрежата
#define MQTT_BROKER_PORT 1883
#define MQTT_CLIENT_ID_PREFIX "smartcamper_"

// MQTT топици
#define MQTT_TOPIC_PREFIX "smartcamper/"
#define MQTT_TOPIC_SENSORS "smartcamper/sensors/"
#define MQTT_TOPIC_COMMANDS "smartcamper/commands/"

// Timing настройки
#define SENSOR_READ_INTERVAL 1000  // 1 секунда
#define HEARTBEAT_INTERVAL 10000   // 10 секунди - гарантирано изпращане
#define MQTT_RECONNECT_DELAY 5000  // 5 секунди
#define WIFI_RECONNECT_DELAY 10000 // 10 секунди

// Sensor threshold настройки
#define TEMP_THRESHOLD 0.1     // 0.1°C промяна
#define HUMIDITY_THRESHOLD 1.0 // 1% промяна

// Debug настройки
#define DEBUG_SERIAL true
#define DEBUG_MQTT true

#endif
