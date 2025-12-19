// Network Manager Implementation
// Universal WiFi manager for ESP32 modules

#include "NetworkManager.h"

NetworkManager::NetworkManager() {
  this->ssid = WIFI_SSID;
  this->password = WIFI_PASSWORD;
  this->lastReconnectAttempt = 0;
  this->lastWiFiCheck = 0;
  this->isConnected = false;
}

NetworkManager::NetworkManager(String ssid, String password) {
  this->ssid = ssid;
  this->password = password;
  this->lastReconnectAttempt = 0;
  this->lastWiFiCheck = 0;
  this->isConnected = false;
}

void NetworkManager::begin() {
  // Don't save SSID to flash (each boot is "clean")
  WiFi.persistent(false);
  
  // Clear all old WiFi entries
  WiFi.disconnect(true, true);  // true,true = clear flash
  delay(500);
  
  // Set mode and settings
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  
  // Ensure we use DHCP (not static IP)
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
  
  if (DEBUG_SERIAL) {
    Serial.println("🔌 Connecting to WiFi: " + ssid);
  }
  
  // Start connection
  WiFi.begin(ssid.c_str(), password.length() > 0 ? password.c_str() : NULL);
  
  // Make first connection attempt (wait up to 10 seconds)
  if (DEBUG_SERIAL) {
    Serial.println("⏳ Waiting for initial connection...");
  }
  
  int initialAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && initialAttempts < 20) {
    delay(500);
    initialAttempts++;
    if (DEBUG_SERIAL && initialAttempts % 5 == 0) {
      Serial.print(".");
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    isConnected = true;
    if (DEBUG_SERIAL) {
      Serial.println();
      Serial.println("✅ Initial WiFi connection successful!");
      Serial.println("IP: " + getLocalIP());
    }
  } else {
    isConnected = false;
    if (DEBUG_SERIAL) {
      Serial.println();
      Serial.println("⚠️ Initial WiFi connection failed, will retry in loop()");
      Serial.println("WiFi Status: " + String(WiFi.status()));
    }
    // Set lastReconnectAttempt so loop() can retry after WIFI_RECONNECT_DELAY
    lastReconnectAttempt = millis();
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
        lastReconnectAttempt = currentTime - WIFI_RECONNECT_DELAY; // Force reconnection attempt
      } else {
        isConnected = true;
      }
    }
  }
  
  if (!isWiFiConnected()) {
    if (currentTime - lastReconnectAttempt > WIFI_RECONNECT_DELAY) {
      lastReconnectAttempt = currentTime;
      connect();
    }
  }
}

bool NetworkManager::connect() {
  // Don't save SSID to flash
  WiFi.persistent(false);
  
  // Clear old entries before each attempt
  WiFi.disconnect(true, true);
  delay(500);
  
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
  
  if (DEBUG_SERIAL) {
    Serial.println("🔄 Attempting WiFi connection...");
  }
  
  // Start connection
  WiFi.begin(ssid.c_str(), password.length() > 0 ? password.c_str() : NULL);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {  // Increase to 30 attempts (15 seconds)
    delay(500);
    attempts++;
    if (DEBUG_SERIAL) {
      Serial.print(".");
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    isConnected = true;
    if (DEBUG_SERIAL) {
      Serial.println();
      Serial.println("✅ WiFi connected!");
      Serial.println("IP: " + getLocalIP());
      Serial.println("Gateway: " + WiFi.gatewayIP().toString());
      Serial.println("DNS: " + WiFi.dnsIP().toString());
    }
    return true;
  } else {
    isConnected = false;
    if (DEBUG_SERIAL) {
      Serial.println();
      Serial.println("❌ WiFi connection failed");
      Serial.println("WiFi Status: " + String(WiFi.status()));
      Serial.println("Local IP: " + WiFi.localIP().toString());
    }
    return false;
  }
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
