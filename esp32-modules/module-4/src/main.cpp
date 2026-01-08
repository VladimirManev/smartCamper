/**
 * @file main.cpp
 * @brief Module 4 - Main Entry Point
 * 
 * ESP32 module with base infrastructure ready for extension.
 * This module can be extended with specific sensors/actuators as needed.
 * 
 * Architecture:
 * - ModuleManager: Handles common infrastructure (Network, MQTT, Heartbeat, Commands)
 * - SensorManager: Base coordinator (can be extended with specific functionality)
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
 * 2. Sensor manager (base implementation)
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
  
  // Initialize sensor manager
  sensorManager.begin();
  
  if (DEBUG_SERIAL) {
    Serial.println("✅ Module 4 fully initialized and ready!");
  }
}

/**
 * @brief Main loop - called repeatedly
 * 
 * Update order is important:
 * 1. Infrastructure first (Network, MQTT, Heartbeat)
 * 2. Sensor manager second (can be extended with specific functionality)
 */
void loop() {
  // Update infrastructure (Network, MQTT, Heartbeat)
  // This must be called first to ensure connectivity
  moduleManager.loop();
  
  // Only update sensor manager if infrastructure is connected
  if (moduleManager.isConnected()) {
    // Update sensor manager (can be extended with specific functionality)
    sensorManager.loop();
  }
  // If not connected, sensor manager will wait until connection is restored
}

