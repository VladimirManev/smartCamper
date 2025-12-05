// ESP32 Common Configuration
// Обща конфигурация за LED Controller модул

#ifndef CONFIG_H
#define CONFIG_H

// WiFi настройки
#define WIFI_SSID "SmartCamper"
#define WIFI_PASSWORD "12344321"

// MQTT настройки - ПРОМЕНИ НА IP НА КОМПЮТЪРА КЪДЕТО РАБОТИ BACKEND!
#define MQTT_BROKER_IP "192.168.4.1"  // Pi IP в SmartCamper мрежата
#define MQTT_BROKER_PORT 1883
#define MQTT_CLIENT_ID_PREFIX "smartcamper_led_"

// MQTT топици
#define MQTT_TOPIC_PREFIX "smartcamper/"
#define MQTT_TOPIC_SENSORS "smartcamper/sensors/"
#define MQTT_TOPIC_COMMANDS "smartcamper/commands/"

// Hardware настройки
#define NUM_RELAYS 1     // Number of relays (easily expandable)

// Timing настройки
#define MQTT_RECONNECT_DELAY 2000  // 2 секунди
#define WIFI_RECONNECT_DELAY 3000  // 3 секунди
#define WIFI_CHECK_INTERVAL 2000   // 2 секунди - проверка на WiFi връзката
#define WIFI_PING_TIMEOUT 1000     // 1 секунда timeout за ping
#define STATUS_PUBLISH_INTERVAL 5000  // 5 секунди - публикуване на статус (deprecated, не се използва)
#define HEARTBEAT_INTERVAL 10000  // 10 секунди - интервал за изпращане на пълен статус

// Debug настройки
#define DEBUG_SERIAL true
#define DEBUG_MQTT true
#define DEBUG_VERBOSE false  // Подробни съобщения (Kitchen sync, детайли)

#endif

