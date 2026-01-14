// ESP32 Common Configuration
// Common configuration for all ESP32 modules

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
#define MODULE_ID "module-3" // Unique identifier for this ESP32 module

// MQTT topics
#define MQTT_TOPIC_PREFIX "smartcamper/"
#define MQTT_TOPIC_SENSORS "smartcamper/sensors/"
#define MQTT_TOPIC_COMMANDS "smartcamper/commands/"

// Timing settings
#define HEARTBEAT_INTERVAL 10000  // 10 seconds - guaranteed send
#define MQTT_RECONNECT_DELAY 2000 // 2 seconds (reduced from 5s)
#define WIFI_RECONNECT_DELAY 3000 // 3 seconds (reduced from 10s)
#define WIFI_CHECK_INTERVAL 2000  // 2 seconds - WiFi connection check
#define WIFI_PING_TIMEOUT 1000    // 1 second timeout for ping

// Floor Heating Configuration (Module 3 specific)
#define NUM_HEATING_CIRCLES 4 // Number of heating circles

// Heating circle names (for identification)
// Circle 0: Central 1
// Circle 1: Central 2
// Circle 2: Bathroom
// Circle 3: Podium

// Temperature sensor pins (DS18B20 - OneWire)
#define HEATING_TEMP_PIN_0 25 // GPIO pin for Circle 0 temperature sensor
#define HEATING_TEMP_PIN_1 26 // GPIO pin for Circle 1 temperature sensor
#define HEATING_TEMP_PIN_2 27 // GPIO pin for Circle 2 temperature sensor
#define HEATING_TEMP_PIN_3 33 // GPIO pin for Circle 3 temperature sensor

// Relay pins (for controlling heating circles)
#define HEATING_RELAY_PIN_0 4  // GPIO pin for Circle 0 relay
#define HEATING_RELAY_PIN_1 5  // GPIO pin for Circle 1 relay
#define HEATING_RELAY_PIN_2 18 // GPIO pin for Circle 2 relay
#define HEATING_RELAY_PIN_3 19 // GPIO pin for Circle 3 relay

// Button pins (for manual control - toggle buttons)
#define HEATING_BUTTON_PIN_0 12 // GPIO pin for Circle 0 button
#define HEATING_BUTTON_PIN_1 13 // GPIO pin for Circle 1 button
#define HEATING_BUTTON_PIN_2 14 // GPIO pin for Circle 2 button
#define HEATING_BUTTON_PIN_3 15 // GPIO pin for Circle 3 button

// Temperature control settings
#define HEATING_TARGET_TEMP 33.0       // Target temperature (default, will be configurable in future)
#define HEATING_HYSTERESIS 2.0         // Hysteresis: 2°C (turn off at 33°C, turn on at 32°C)
#define HEATING_TURN_OFF_TEMP 33.0     // Turn off relay when temperature reaches this
#define HEATING_TURN_ON_TEMP 32.0      // Turn on relay when temperature drops below this
#define HEATING_MEASURE_INTERVAL 30000 // 30 seconds - temperature measurement interval

// Temperature sensor settings (DS18B20)
#define HEATING_TEMP_READ_INTERVAL 5000    // 5 seconds - measurement interval
#define HEATING_TEMP_AVERAGE_INTERVAL 30000 // 30 seconds - average calculation interval
#define HEATING_TEMP_THRESHOLD 0.1         // 0.1°C change threshold for publishing
#define HEATING_TEMP_AVERAGE_COUNT 6       // Number of measurements to average (6 measurements = 30 seconds)

// Leveling Sensor Configuration (Module 3 specific - GY-521 MPU6050)
#define LEVELING_I2C_SDA 21      // GPIO pin for I²C SDA (Wire interface)
#define LEVELING_I2C_SCL 22      // GPIO pin for I²C SCL (Wire interface)
#define LEVELING_READ_INTERVAL 500  // 500ms (0.5 seconds) - measurement and publish interval when active
#define LEVELING_TIMEOUT 22000   // 22 seconds - timeout to stop publishing and measuring
#define LEVELING_ZERO_BUTTON_PIN 0  // GPIO 0 - BOOT button for zeroing leveling
#define LEVELING_ZERO_BUTTON_HOLD_TIME 3000  // 3 seconds - hold time to zero leveling
#define LEVELING_LED_PIN 2       // GPIO 2 - Built-in LED for visual feedback

// Circle modes (used by FloorHeatingController and FloorHeatingSensor)
enum CircleMode
{
  CIRCLE_MODE_OFF = 0,         // Circle is off, no temperature control
  CIRCLE_MODE_TEMP_CONTROL = 1 // Circle is in temperature control mode
};

// Debug settings
#define DEBUG_SERIAL true   // Enable serial debug output
#define DEBUG_MQTT true     // Enable MQTT debug output
#define DEBUG_VERBOSE false // Enable verbose debug output (set to true for detailed logging)

// Watchdog settings (optional - for production stability)
// #define ENABLE_WATCHDOG true
// #define WATCHDOG_TIMEOUT 30  // seconds

#endif
