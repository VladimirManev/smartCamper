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
#include <esp_system.h>
#include <esp_task_wdt.h>

// Global managers - initialized in setup()
ModuleManager moduleManager;
SensorManager sensorManager(&moduleManager);

// Memory monitoring
unsigned long lastMemoryLog = 0;
const unsigned long MEMORY_LOG_INTERVAL = 300000;  // 5 minutes

/**
 * @brief Print reset reason for debugging
 */
void printResetReason() {
  if (!DEBUG_SERIAL) {
    return;
  }
  
  Serial.println("\n🔍 === RESET DIAGNOSTICS ===");
  
  // Get reset reason
  esp_reset_reason_t resetReason = esp_reset_reason();
  
  Serial.print("Reset Reason: ");
  switch (resetReason) {
    case ESP_RST_UNKNOWN:
      Serial.println("UNKNOWN");
      break;
    case ESP_RST_POWERON:
      Serial.println("POWERON (normal boot)");
      break;
    case ESP_RST_EXT:
      Serial.println("EXTERNAL (reset pin)");
      break;
    case ESP_RST_SW:
      Serial.println("SOFTWARE (esp_restart())");
      break;
    case ESP_RST_PANIC:
      Serial.println("PANIC (exception/assert)");
      break;
    case ESP_RST_INT_WDT:
      Serial.println("INTERRUPT WDT (watchdog timeout)");
      break;
    case ESP_RST_TASK_WDT:
      Serial.println("TASK WDT (task watchdog timeout)");
      break;
    case ESP_RST_WDT:
      Serial.println("OTHER WDT (other watchdog)");
      break;
    case ESP_RST_DEEPSLEEP:
      Serial.println("DEEPSLEEP");
      break;
    case ESP_RST_BROWNOUT:
      Serial.println("BROWNOUT (low voltage)");
      break;
    case ESP_RST_SDIO:
      Serial.println("SDIO");
      break;
    default:
      Serial.println("UNKNOWN CODE: " + String(resetReason));
      break;
  }
  
  // Get free heap
  Serial.println("Free Heap: " + String(ESP.getFreeHeap()) + " bytes");
  Serial.println("Largest Free Block: " + String(ESP.getMaxAllocHeap()) + " bytes");
  Serial.println("Min Free Heap (ever): " + String(ESP.getMinFreeHeap()) + " bytes");
  
  // Get chip info
  Serial.println("Chip Model: " + String(ESP.getChipModel()));
  Serial.println("CPU Frequency: " + String(ESP.getCpuFreqMHz()) + " MHz");
  
  Serial.println("===========================\n");
}

/**
 * @brief Setup function - called once on boot
 * 
 * Initializes:
 * 1. Module infrastructure (Network, MQTT, Heartbeat, Commands)
 * 2. Sensor manager (base implementation)
 */
void setup() {
  // Start serial first for diagnostics
  Serial.begin(115200);
  delay(100);  // Give serial time to initialize
  
  // Print reset reason for debugging
  printResetReason();
  
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
  // Feed watchdog to prevent timeout (ESP32 task watchdog)
  // This ensures loop() doesn't block for too long
  yield();  // Give other tasks a chance to run
  
  // Update infrastructure (Network, MQTT, Heartbeat)
  // This must be called first to ensure connectivity
  moduleManager.loop();
  
  // ALWAYS update sensor manager (physical functions must work offline)
  // Status publishing will only happen if connected (handled inside SensorManager)
  sensorManager.loop();
  
  // Periodic memory monitoring (every 5 minutes)
  unsigned long currentTime = millis();
  if (DEBUG_SERIAL && (currentTime - lastMemoryLog > MEMORY_LOG_INTERVAL)) {
    lastMemoryLog = currentTime;
    Serial.println("📊 Memory Status:");
    Serial.println("  Free Heap: " + String(ESP.getFreeHeap()) + " bytes");
    Serial.println("  Min Free Heap: " + String(ESP.getMinFreeHeap()) + " bytes");
    Serial.println("  Largest Free Block: " + String(ESP.getMaxAllocHeap()) + " bytes");
    Serial.println("  Uptime: " + String(currentTime / 1000) + " seconds");
    
    // Warning if heap is getting low
    if (ESP.getFreeHeap() < 20000) {
      Serial.println("⚠️ WARNING: Low heap memory!");
    }
  }
}

