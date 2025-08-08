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

// I2C настройки за MPU6050
#define I2C_SDA 21     // SDA пин за MPU6050
#define I2C_SCL 22     // SCL пин за MPU6050

// MQTT Topics
#define MQTT_TOPIC_TEMPERATURE "smartcamper/sensors/temperature/living/data"
#define MQTT_TOPIC_HUMIDITY "smartcamper/sensors/humidity/living/data"
#define MQTT_TOPIC_TILT "smartcamper/sensors/tilt/living/data"

// Интервали
#define WIFI_RETRY_INTERVAL 5000    // 5 секунди между опитите за WiFi
#define MQTT_KEEPALIVE 60           // 60 секунди keep-alive за MQTT

// Debug настройки
#define DEBUG_SERIAL true // Включване на debug съобщения

#endif