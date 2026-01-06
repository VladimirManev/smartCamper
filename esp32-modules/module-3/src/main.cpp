/**
 * @file main.cpp
 * @brief Module 3 - Main Entry Point
 * 
 * ESP32 module for floor heating control with automatic temperature management.
 * 
 * Architecture:
 * - ModuleManager: Handles common infrastructure (Network, MQTT, Heartbeat, Commands)
 * - FloorHeatingManager: Coordinates all floor heating functionality
 * - FloorHeatingController: Manages relay control and temperature-based automatic control
 * - FloorHeatingSensor: Temperature sensors for each heating circle (DS18B20)
 * - FloorHeatingButtonHandler: Handles button input for manual control
 */

#include "Config.h"
#include "ModuleManager.h"
#include "FloorHeatingManager.h"

// Global managers - initialized in setup()
ModuleManager moduleManager;
FloorHeatingManager floorHeatingManager(&moduleManager);

/**
 * @brief Setup function - called once on boot
 * 
 * Initializes:
 * 1. Module infrastructure (Network, MQTT, Heartbeat, Commands)
 * 2. Floor heating components (controller, sensors, buttons)
 */
void setup() {
  // Initialize module infrastructure (Network, MQTT, Heartbeat, Commands)
  // CommandHandler is passed to ModuleManager for MQTT command processing
  moduleManager.begin(&floorHeatingManager.getCommandHandler());
  
  // Verify initialization
  if (!moduleManager.isInitialized()) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ ERROR: ModuleManager failed to initialize!");
    }
    // In production, could trigger watchdog reset here
    return;
  }
  
  // Initialize floor heating components
  floorHeatingManager.begin();
  
  if (DEBUG_SERIAL) {
    Serial.println("✅ Module 3 fully initialized and ready!");
    Serial.println("Floor heating system with " + String(NUM_HEATING_CIRCLES) + " circles");
    Serial.println("Target temperature: " + String(HEATING_TARGET_TEMP) + "°C");
    Serial.println("Hysteresis: " + String(HEATING_HYSTERESIS) + "°C");
  }
}

/**
 * @brief Main loop - called repeatedly
 * 
 * Update order is important:
 * 1. Infrastructure first (Network, MQTT, Heartbeat)
 * 2. Floor heating components second (sensors, controller, buttons)
 * Note: Floor heating works offline - sensors and controller always run
 */
void loop() {
  // Update infrastructure (Network, MQTT, Heartbeat)
  // This must be called first to ensure connectivity
  moduleManager.loop();
  
  // ALWAYS update floor heating components (sensors, controller, buttons) - physical functions must work offline
  // Status publishing will only happen if connected (handled inside FloorHeatingManager)
  floorHeatingManager.loop();
  
  // No delay() - use yield() if needed, but ESP32 handles task switching automatically
  // delay() blocks all operations including button handling
}

