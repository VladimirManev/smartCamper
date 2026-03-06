/**
 * @file main.cpp
 * @brief Module 5 - Main Entry Point
 * 
 * ESP32 module for appliance control (Audio System, Water Pump, Refrigerator).
 * 
 * Architecture:
 * - ModuleManager: Handles common infrastructure (Network, MQTT, Heartbeat, Commands)
 * - ApplianceManager: Coordinates all appliance-related functionality
 * - RelayController: Manages relay control for appliances
 * - ButtonHandler: Handles button input processing
 */

#include "Config.h"
#include "ModuleManager.h"
#include "ApplianceManager.h"

// Global managers - initialized in setup()
ModuleManager moduleManager;
ApplianceManager applianceManager(&moduleManager);

/**
 * @brief Setup function - called once on boot
 * 
 * Initializes:
 * 1. Module infrastructure (Network, MQTT, Heartbeat, Commands)
 * 2. Appliance-related components (relays, buttons)
 */
void setup() {
  // Initialize module infrastructure (Network, MQTT, Heartbeat, Commands)
  // CommandHandler is passed to ModuleManager for MQTT command processing
  // Note: ApplianceManager will set its own MQTT callback in begin() for appliance-specific commands
  moduleManager.begin(&applianceManager.getCommandHandler());
  
  // Verify initialization
  if (!moduleManager.isInitialized()) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ ERROR: ModuleManager failed to initialize!");
    }
    // In production, could trigger watchdog reset here
    return;
  }
  
  // Initialize appliance components
  applianceManager.begin();
  
  if (DEBUG_SERIAL) {
    Serial.println("✅ Module 5 fully initialized and ready!");
    Serial.println("Click: Toggle appliance ON/OFF\n");
  }
}

/**
 * @brief Main loop - called repeatedly
 * 
 * Update order is important:
 * 1. Infrastructure first (Network, MQTT, Heartbeat)
 * 2. Appliance components second (relays, buttons)
 */
void loop() {
  // Update infrastructure (Network, MQTT, Heartbeat)
  // This must be called first to ensure connectivity
  moduleManager.loop();
  
  // ALWAYS update appliance components (relays, buttons) - physical functions must work offline
  // Status publishing will only happen if connected (handled inside ApplianceManager)
  applianceManager.loop();
  
  delay(10);  // Small delay to prevent overwhelming the system
}
