// ESP32 Module 6 Configuration
// Victron BLE energy monitor + AAOBOSI / Alpicool fridge GATT client

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
#define MODULE_ID "module-6"

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

// Victron BLE settings
#define VICTRON_STATUS_PUBLISH_INTERVAL_MS 2000 // Full status every 2 seconds
#define BLE_SCAN_INTERVAL_MS 100
#define BLE_SCAN_WINDOW_MS 100 // Full window = best capture; BLE starts after WiFi connect
#define BLE_SCAN_BURST_SEC 5   // Non-blocking scan burst, restarted from loop when idle

// AAOBOSI / Alpicool fridge (GATT client; Victron owns BLEScan callbacks)
#define FRIDGE_ENABLED true
#define FRIDGE_BLE_MAC "22:2a:06:a6:f3:48"
#define FRIDGE_QUERY_INTERVAL_MS 2000
#define FRIDGE_RECONNECT_INTERVAL_MS 8000
#define FRIDGE_QUERY_RESPONSE_MS 1800
// Quiet Victron scan around fridge commands / write-path probe
#define FRIDGE_CMD_RADIO_QUIET_MS 500

// Optional local GPIO buttons (INPUT_PULLUP, press = pin to GND). Safe if unused.
#define FRIDGE_BTN_POWER 32
#define FRIDGE_BTN_Z1_UP 33
#define FRIDGE_BTN_Z1_DOWN 25
#define FRIDGE_BTN_Z2_UP 26
#define FRIDGE_BTN_Z2_DOWN 27
#define FRIDGE_BTN_DEBOUNCE_MS 40
#define FRIDGE_TEMP_MIN_C (-20)
#define FRIDGE_TEMP_MAX_C 20

// AC charger (Blue Smart / Phoenix)
#define AC_CHARGER_ENABLED true
#define AC_CHARGER_MAC "CF:82:A4:8F:EA:04"
#define AC_CHARGER_KEY "619366844ff9d1adbff0669f3cfd62ca"

// Victron device credentials from VictronConnect:
// Settings -> Product Info -> Instant Readout via Bluetooth -> Show
#if AC_CHARGER_ENABLED
#define VICTRON_DEVICE_COUNT 5
#else
#define VICTRON_DEVICE_COUNT 4
#endif

#define VICTRON_DEVICE_0_NAME "SmartShunt"
#define VICTRON_DEVICE_0_MAC "E7:47:43:C9:5D:09"
#define VICTRON_DEVICE_0_KEY "af621f5c41707664f6b5ef35ad0d2b99"

#define VICTRON_DEVICE_1_NAME "Orion"
#define VICTRON_DEVICE_1_MAC "E8:42:AE:38:C1:C6"
#define VICTRON_DEVICE_1_KEY "cefa9834a87e10fc946b39e644d00998"

#define VICTRON_DEVICE_2_NAME "MPPT1"
#define VICTRON_DEVICE_2_MAC "D3:AD:2A:CC:47:8C"
#define VICTRON_DEVICE_2_KEY "da5ba206e2dc1dc7c6d7d5516dc350b0"

#define VICTRON_DEVICE_3_NAME "MPPT2"
#define VICTRON_DEVICE_3_MAC "DC:41:88:BE:96:18"
#define VICTRON_DEVICE_3_KEY "ac0f048ff4ff8c411f2c5eb05bc42a5e"

// Debug settings
#define DEBUG_SERIAL true
#define DEBUG_MQTT false
#define DEBUG_VERBOSE false

#endif
