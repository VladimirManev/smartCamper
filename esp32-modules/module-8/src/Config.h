// ESP32 Module 8 Configuration
// Security alarm module (interior + perimeter)

#ifndef CONFIG_H
#define CONFIG_H

// WiFi settings
#define WIFI_SSID "SmartCamper"
#define WIFI_PASSWORD "12344321"

// MQTT settings
#define MQTT_BROKER_IP "192.168.4.1"
#define MQTT_BROKER_PORT 1883
#define MQTT_CLIENT_ID_PREFIX "smartcamper_"

// Module identification
#define MODULE_ID "module-8"

// MQTT topics
#define MQTT_TOPIC_PREFIX "smartcamper/"
#define MQTT_TOPIC_SENSORS "smartcamper/sensors/"
#define MQTT_TOPIC_COMMANDS "smartcamper/commands/"

// Timing — network
#define HEARTBEAT_INTERVAL 10000
#define MQTT_RECONNECT_DELAY 2000
#define WIFI_RECONNECT_DELAY 3000
#define WIFI_CHECK_INTERVAL 2000
#define WIFI_PING_TIMEOUT 1000

// Disarm PIN (MQTT zone/1/off only) — change requires reflash
#define ALARM_DISARM_PIN "1234"

// --- GPIO pins ---
// BOOT button on most ESP32 DevKit boards
#define ALARM_BUTTON_PIN 0

// Spare contact (INPUT_PULLUP, LOW = open) — hood / door sim for now
#define SPARE_CONTACT_PIN 4

// Interior PIR (HIGH = motion)
#define INTERIOR_PIR_PIN 5

// Exterior PIR (HIGH = motion)
#define PERIMETER_PIR_FRONT 18
#define PERIMETER_PIR_REAR 19
#define PERIMETER_PIR_LEFT_FRONT 21
#define PERIMETER_PIR_LEFT_REAR 22
#define PERIMETER_PIR_RIGHT_FRONT 23
#define PERIMETER_PIR_RIGHT_REAR 25
#define NUM_PERIMETER_PIRS 6

// Outputs
#define BUZZER_PIN 26
#define SIREN_RELAY_PIN 27
#define SMOKE_RELAY_PIN 32
#define ZONE1_LED_PIN 33

// Ducato B-CAN (WCMCU-230, OBD pins 1+9, 50 kbit/s listen-only)
#define CAN_TX_PIN 17
#define CAN_RX_PIN 16
#define CAN_DOOR_ID 0x06214000UL
#define CAN_DOOR_BYTE 1
#define CAN_MASK_DRIVER 0x04
#define CAN_MASK_PASSENGER 0x08
#define CAN_MASK_SLIDING 0x30
#define CAN_MASK_REAR 0x40

// --- Button sequence timing ---
#define BUTTON_SHORT_MAX_MS 500
#define BUTTON_TAP_GAP_MAX_MS 800
#define BUTTON_HOLD_MIN_MS 3000
#define BUTTON_DEBOUNCE_MS 50

// --- Alarm timing ---
#define EXIT_DELAY_MS 30000
#define ENTRY_DELAY_MS 30000
#define SIREN_DURATION_MS 120000
#define SMOKE_START_AFTER_SIREN_MS 10000
#define SMOKE_DURATION_MS 60000

// Delay buzzer phases (exit + entry) — totals EXIT/ENTRY delay
#define DELAY_PHASE1_MS 3000  // 1 beep / s
#define DELAY_PHASE2_MS 3000  // 2 beeps / s
#define DELAY_PHASE3_MS 3000  // 6 beeps / s
#define DELAY_PHASE3_PERIOD_MS 167  // ~1000/6

// Perimeter alert
#define PERIMETER_BEEP_COUNT 5
#define PERIMETER_REPEAT_MS 8000
#define PERIMETER_DEBOUNCE_MS 120  // require stable HIGH/LOW (filters floating chatter)

// LED blink
#define ZONE1_LED_BLINK_MS 500
#define ZONE1_LED_CAT_ON_MS 70
#define ZONE1_LED_CAT_OFF_MS 70
#define ZONE1_LED_CAT_PAUSE_MS 450

// Ignore sensor edges after boot (PIR / floating pins settle)
#define INPUT_SETTLE_MS 2000

// Buzzer tone lengths
#define BEEP_SHORT_MS 80
#define BEEP_LONG_MS 400
#define BEEP_GAP_MS 100
#define CONFIRM_PAUSE_MS 1000
#define ERROR_BEEP_MS 600

// Debug
#define DEBUG_SERIAL true
#define DEBUG_MQTT false
#define DEBUG_VERBOSE false

#endif
