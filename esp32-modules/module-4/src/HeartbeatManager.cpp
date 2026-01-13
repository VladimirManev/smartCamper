// Heartbeat Manager Implementation
// Standalone heartbeat functionality for ESP32 modules

#include "HeartbeatManager.h"
#include "MQTTManager.h"
#include <ArduinoJson.h>
#include <WiFi.h>
#include <esp_system.h>

// MQTT topic prefix for heartbeats
#define HEARTBEAT_TOPIC_PREFIX "smartcamper/heartbeat/"

HeartbeatManager::HeartbeatManager(MQTTManager* mqtt, String moduleId) {
  this->mqttManager = mqtt;
  this->moduleId = moduleId;
  this->lastHeartbeatSent = 0;
  this->enabled = true;
  this->uptimeStart = millis();
  this->lastMQTTState = false;  // Initialize as disconnected
  this->resetReasonSent = false;
  
  // Get reset reason once at boot
  esp_reset_reason_t resetReason = esp_reset_reason();
  switch (resetReason) {
    case ESP_RST_UNKNOWN:
      this->resetReason = "UNKNOWN";
      break;
    case ESP_RST_POWERON:
      this->resetReason = "POWERON";
      break;
    case ESP_RST_EXT:
      this->resetReason = "EXTERNAL";
      break;
    case ESP_RST_SW:
      this->resetReason = "SOFTWARE";
      break;
    case ESP_RST_PANIC:
      this->resetReason = "PANIC";
      break;
    case ESP_RST_INT_WDT:
      this->resetReason = "INT_WDT";
      break;
    case ESP_RST_TASK_WDT:
      this->resetReason = "TASK_WDT";
      break;
    case ESP_RST_WDT:
      this->resetReason = "WDT";
      break;
    case ESP_RST_DEEPSLEEP:
      this->resetReason = "DEEPSLEEP";
      break;
    case ESP_RST_BROWNOUT:
      this->resetReason = "BROWNOUT";
      break;
    case ESP_RST_SDIO:
      this->resetReason = "SDIO";
      break;
    default:
      this->resetReason = "UNKNOWN_" + String(resetReason);
      break;
  }
}

void HeartbeatManager::begin() {
  if (DEBUG_SERIAL) {
    Serial.println("💓 Heartbeat Manager initialized");
    Serial.println("   Module ID: " + moduleId);
    Serial.println("   Topic: " + String(HEARTBEAT_TOPIC_PREFIX) + moduleId);
  }
}

void HeartbeatManager::loop() {
  bool mqttConnected = mqttManager->isMQTTConnected();
  
  // Detect MQTT reconnection (transition from disconnected to connected)
  if (mqttConnected && !lastMQTTState) {
    // MQTT just connected/reconnected - send heartbeat immediately
    if (enabled) {
      if (DEBUG_SERIAL) {
        Serial.println("💓 MQTT reconnected - sending immediate heartbeat");
      }
      sendHeartbeat();
    }
  }
  
  // Update last known MQTT state
  lastMQTTState = mqttConnected;
  
  // Only send periodic heartbeat if enabled and MQTT is connected
  if (!enabled || !mqttConnected) {
    return;
  }
  
  // Check if it's time to send periodic heartbeat
  if (shouldSendHeartbeat()) {
    sendHeartbeat();
  }
}

bool HeartbeatManager::shouldSendHeartbeat() {
  unsigned long currentTime = millis();
  unsigned long timeSinceLastHeartbeat = currentTime - lastHeartbeatSent;
  
  // Send heartbeat if interval has elapsed
  return (timeSinceLastHeartbeat >= HEARTBEAT_INTERVAL);
}

void HeartbeatManager::sendHeartbeat() {
  String payload = buildHeartbeatPayload();
  String topic = String(HEARTBEAT_TOPIC_PREFIX) + moduleId;
  
  // Publish heartbeat via MQTT manager using raw publish
  bool success = mqttManager->publishRaw(topic, payload);
  
  if (success) {
    lastHeartbeatSent = millis();
    
    if (DEBUG_MQTT) {
      Serial.println("💓 Heartbeat sent: " + topic);
      Serial.println("   Payload: " + payload);
    }
  } else {
    if (DEBUG_SERIAL) {
      Serial.println("❌ Failed to send heartbeat: " + topic);
    }
  }
}

String HeartbeatManager::buildHeartbeatPayload() {
  // Create JSON payload
  StaticJsonDocument<256> doc;
  
  // Timestamp (milliseconds since boot, converted to seconds for consistency)
  doc["timestamp"] = millis() / 1000;
  
  // Module identifier
  doc["moduleId"] = moduleId;
  
  // Uptime in seconds
  doc["uptime"] = getUptimeSeconds();
  
  // WiFi signal strength (if connected)
  if (WiFi.status() == WL_CONNECTED) {
    doc["wifiRSSI"] = WiFi.RSSI();
  } else {
    doc["wifiRSSI"] = -999;  // Invalid value to indicate no WiFi
  }
  
  // Reset reason - only include in first heartbeat after boot
  if (!resetReasonSent && resetReason.length() > 0) {
    doc["resetReason"] = resetReason;
    resetReasonSent = true;  // Mark as sent, won't include in future heartbeats
  }
  
  // Serialize to string
  String payload;
  serializeJson(doc, payload);
  
  return payload;
}

unsigned long HeartbeatManager::getUptimeSeconds() const {
  return (millis() - uptimeStart) / 1000;
}

void HeartbeatManager::setModuleId(String id) {
  moduleId = id;
  if (DEBUG_SERIAL) {
    Serial.println("💓 Heartbeat module ID changed to: " + moduleId);
  }
}

void HeartbeatManager::setEnabled(bool enabled) {
  this->enabled = enabled;
  if (DEBUG_SERIAL) {
    Serial.println("💓 Heartbeat " + String(enabled ? "enabled" : "disabled"));
  }
}

void HeartbeatManager::forceSend() {
  if (!enabled || !mqttManager->isMQTTConnected()) {
    if (DEBUG_SERIAL) {
      Serial.println("⚠️ Cannot force send heartbeat - " + 
                     String(!enabled ? "disabled" : "MQTT not connected"));
    }
    return;
  }
  
  sendHeartbeat();
}

bool HeartbeatManager::isEnabled() const {
  return enabled;
}

unsigned long HeartbeatManager::getLastSentTime() const {
  return lastHeartbeatSent;
}

String HeartbeatManager::getModuleId() const {
  return moduleId;
}

void HeartbeatManager::printStatus() const {
  if (DEBUG_SERIAL) {
    Serial.println("📊 Heartbeat Manager Status:");
    Serial.println("   Module ID: " + moduleId);
    Serial.println("   Enabled: " + String(enabled ? "Yes" : "No"));
    Serial.println("   MQTT Connected: " + String(mqttManager->isMQTTConnected() ? "Yes" : "No"));
    Serial.println("   Last Sent: " + String((millis() - lastHeartbeatSent) / 1000) + " seconds ago");
    Serial.println("   Uptime: " + String(getUptimeSeconds()) + " seconds");
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("   WiFi RSSI: " + String(WiFi.RSSI()) + " dBm");
    }
  }
}

