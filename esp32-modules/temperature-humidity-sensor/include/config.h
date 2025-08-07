#ifndef CONFIG_H
#define CONFIG_H

// WiFi настройки
#define WIFI_SSID "Zaqci"
#define WIFI_PASSWORD "12344321"

// MQTT настройки
#define MQTT_SERVER "192.168.1.213" // IP адрес на компютъра
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "esp32_temp_humidity_01"
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""

// Сензорни настройки
#define DHT_PIN 25     // Пин за AM2301 сензора
#define DHT_TYPE DHT22 // Тип сензор (AM2301 = DHT22)

// MQTT Topics
#define MQTT_TOPIC_TEMPERATURE "smartcamper/sensors/temperature/living/data"
#define MQTT_TOPIC_HUMIDITY "smartcamper/sensors/humidity/living/data"

// Интервали
#define SENSOR_READ_INTERVAL 5000   // 5 секунди между четенията
#define MQTT_PUBLISH_INTERVAL 10000 // 10 секунди между публикациите
#define WIFI_RETRY_INTERVAL 5000    // 5 секунди между опитите за WiFi

// Debug настройки
#define DEBUG_SERIAL true // Включване на debug съобщения

#endif