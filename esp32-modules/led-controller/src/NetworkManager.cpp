// Network Manager Implementation
// Universal WiFi manager for ESP32 modules

#include "NetworkManager.h"

NetworkManager::NetworkManager() {
  this->ssid = WIFI_SSID;
  this->password = WIFI_PASSWORD;
  this->lastReconnectAttempt = 0;
  this->lastWiFiCheck = 0;
  this->isConnected = false;
  this->disconnectPending = false;
}

NetworkManager::NetworkManager(String ssid, String password) {
  this->ssid = ssid;
  this->password = password;
  this->lastReconnectAttempt = 0;
  this->lastWiFiCheck = 0;
  this->isConnected = false;
  this->disconnectPending = false;
}

void NetworkManager::begin() {
  // Don't save SSID to flash (each boot is "clean")
  WiFi.persistent(false);
  
  // Clear all old WiFi entries (non-blocking - only once)
  WiFi.disconnect(true, true);  // true,true = clear flash
  // Do NOT use delay() here - non-blocking
  
  // Set mode and settings
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  
  // Ensure we use DHCP (not static IP)
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
  
  if (DEBUG_SERIAL) {
    Serial.println("🔌 Connecting to WiFi: " + ssid);
  }
  
  // Start connection (non-blocking)
  WiFi.begin(ssid.c_str(), password.length() > 0 ? password.c_str() : NULL);
  
  // Do NOT wait here - we'll check status in loop()
  isConnected = false;
  lastReconnectAttempt = millis();
  
  if (DEBUG_SERIAL) {
    Serial.println("⏳ WiFi connection started, will check status in loop()");
  }
}

void NetworkManager::loop() {
  unsigned long currentTime = millis();
  
  // Active WiFi connection check at intervals
  if (currentTime - lastWiFiCheck > WIFI_CHECK_INTERVAL) {
    lastWiFiCheck = currentTime;
    
    // If WiFi.status() shows connected but ping doesn't work, consider it a dead connection
    if (WiFi.status() == WL_CONNECTED) {
      if (!checkWiFiConnection()) {
        // Connection is dead, force reconnection
        if (DEBUG_SERIAL) {
          Serial.println("⚠️ WiFi connection is dead (no ping response), forcing reconnect");
        }
        isConnected = false;
        WiFi.disconnect();
        disconnectPending = true;  // Mark that we need disconnect before next attempt
        lastReconnectAttempt = currentTime - WIFI_RECONNECT_DELAY; // Force reconnection attempt
      } else {
        isConnected = true;
      }
    }
  }
  
  if (!isWiFiConnected()) {
    if (currentTime - lastReconnectAttempt > WIFI_RECONNECT_DELAY) {
      lastReconnectAttempt = currentTime;
      
      // Check if WiFi connected between attempts (from auto-reconnect)
      if (WiFi.status() == WL_CONNECTED) {
        isConnected = true;
        disconnectPending = false;  // Successful connection, no need to disconnect
        if (DEBUG_SERIAL) {
          Serial.println("✅ WiFi connected (auto-reconnect)!");
          Serial.println("IP: " + getLocalIP());
        }
      } else {
        // Still not connected, make new attempt
        disconnectPending = true;  // Mark that we need disconnect before next attempt
        connect();  // Start attempt (non-blocking)
        
        // Check status immediately (without delay) - rarely will be connected immediately
        if (WiFi.status() == WL_CONNECTED) {
          isConnected = true;
          disconnectPending = false;
          if (DEBUG_SERIAL) {
            Serial.println("✅ WiFi connected!");
            Serial.println("IP: " + getLocalIP());
          }
        } else {
          isConnected = false;
          if (DEBUG_SERIAL) {
            Serial.println("❌ WiFi connection attempt started, checking status in next loop...");
            Serial.println("WiFi Status: " + String(WiFi.status()));
          }
        }
      }
    } else {
      // Still waiting between attempts, but check if we accidentally connected
      if (WiFi.status() == WL_CONNECTED) {
        isConnected = true;
        disconnectPending = false;
        if (DEBUG_SERIAL) {
          Serial.println("✅ WiFi connected!");
          Serial.println("IP: " + getLocalIP());
        }
      }
    }
  } else {
    // Already connected, no need to disconnect
    disconnectPending = false;
  }
}

bool NetworkManager::connect() {
  // Don't save SSID to flash
  WiFi.persistent(false);
  
  // Clear old entries only if needed (before new attempt after failed)
  if (disconnectPending) {
    WiFi.disconnect(true, true);
    disconnectPending = false;
    // Do NOT use delay() - non-blocking
  }
  
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
  
  if (DEBUG_SERIAL) {
    Serial.println("🔄 Attempting WiFi connection...");
  }
  
  // Start connection (non-blocking)
  WiFi.begin(ssid.c_str(), password.length() > 0 ? password.c_str() : NULL);
  
  // Do NOT block here - just start attempt and check status in next loop()
  // Status will be checked in loop() on next iteration
  
  return false;  // Still not connected, will check in loop()
}

void NetworkManager::disconnect() {
  WiFi.disconnect();
  isConnected = false;
  if (DEBUG_SERIAL) {
    Serial.println("📴 WiFi disconnected");
  }
}

bool NetworkManager::isWiFiConnected() {
  return isConnected && (WiFi.status() == WL_CONNECTED);
}

bool NetworkManager::checkWiFiConnection() {
  // Check if we have gateway IP
  IPAddress gateway = WiFi.gatewayIP();
  if (gateway == INADDR_NONE || gateway[0] == 0) {
    return false;
  }
  
  // Check if we have valid local IP
  IPAddress localIP = WiFi.localIP();
  if (localIP == INADDR_NONE || localIP[0] == 0) {
    return false;
  }
  
  // Additional check - if RSSI is reasonable (not -100 dBm)
  int rssi = WiFi.RSSI();
  if (rssi < -90) {
    // Signal is very weak, might be a problem
    if (DEBUG_SERIAL) {
      Serial.println("⚠️ WiFi RSSI is very weak: " + String(rssi) + " dBm");
    }
    // Don't consider it dead connection, but it's a warning
  }
  
  return true;
}

String NetworkManager::getLocalIP() {
  return WiFi.localIP().toString();
}

