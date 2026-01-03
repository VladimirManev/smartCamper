/**
 * @file main.cpp
 * @brief Module 1 - Main Entry Point
 * 
 * ESP32 module with temperature/humidity sensor (DHT22) and extensible sensor support.
 * This module can be extended with additional sensors as needed.
 * 
 * Architecture:
 * - ModuleManager: Handles common infrastructure (Network, MQTT, Heartbeat, Commands)
 * - SensorManager: Coordinates all sensor classes
 * - TemperatureHumiditySensor: DHT22 sensor implementation
 */

#include "Config.h"
#include "ModuleManager.h"
#include "SensorManager.h"

// Global managers - initialized in setup()
ModuleManager moduleManager;
SensorManager sensorManager(&moduleManager);

/**
 * @brief Setup function - called once on boot
 * 
 * Initializes:
 * 1. Module infrastructure (Network, MQTT, Heartbeat, Commands)
 * 2. Sensor classes
 */
void setup() {
  // Initialize module infrastructure (Network, MQTT, Heartbeat, Commands)
  // CommandHandler is passed to ModuleManager for MQTT command processing
  moduleManager.begin(&sensorManager.getCommandHandler());
  
  // Verify initialization
  if (!moduleManager.isInitialized()) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ ERROR: ModuleManager failed to initialize!");
    }
    // In production, could trigger watchdog reset here
    return;
  }
  
  // Initialize sensors
  sensorManager.begin();
  
  if (DEBUG_SERIAL) {
    Serial.println("✅ Module 1 fully initialized and ready!");
  }
}

/**
 * @brief Main loop - called repeatedly
 * 
 * Update order is important:
 * 1. Infrastructure first (Network, MQTT, Heartbeat)
 * 2. Sensors second (read and publish data)
 */
void loop() {
  // Update infrastructure (Network, MQTT, Heartbeat)
  // This must be called first to ensure connectivity
  moduleManager.loop();
  
  // Only update sensors if infrastructure is connected
  if (moduleManager.isConnected()) {
    // Update sensors (read and publish data)
    sensorManager.loop();
  }
  // If not connected, sensors will wait until connection is restored
}
