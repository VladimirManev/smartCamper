// Network Manager Implementation
// Универсален WiFi мениджър за ESP32 модули

#include "NetworkManager.h"

NetworkManager::NetworkManager() {
  this->ssid = WIFI_SSID;
  this->password = WIFI_PASSWORD;
  this->lastReconnectAttempt = 0;
  this->isConnected = false;
}

NetworkManager::NetworkManager(String ssid, String password) {
  this->ssid = ssid;
  this->password = password;
  this->lastReconnectAttempt = 0;
  this->isConnected = false;
}

void NetworkManager::begin() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  
  if (DEBUG_SERIAL) {
    Serial.println("🔌 Connecting to WiFi...");
    Serial.println("SSID: " + ssid);
  }
}

void NetworkManager::loop() {
  unsigned long currentTime = millis();
  
  if (!isWiFiConnected()) {
    if (currentTime - lastReconnectAttempt > WIFI_RECONNECT_DELAY) {
      lastReconnectAttempt = currentTime;
      connect();
    }
  }
}

bool NetworkManager::connect() {
  if (DEBUG_SERIAL) {
    Serial.println("🔄 Attempting WiFi connection...");
  }
  
  WiFi.begin(ssid.c_str(), password.c_str());
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
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
    }
    return true;
  } else {
    isConnected = false;
    if (DEBUG_SERIAL) {
      Serial.println();
      Serial.println("❌ WiFi connection failed");
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
  return WiFi.status() == WL_CONNECTED;
}

String NetworkManager::getLocalIP() {
  return WiFi.localIP().toString();
}

void NetworkManager::printStatus() {
  if (DEBUG_SERIAL) {
    Serial.println("📊 WiFi Status:");
    Serial.println("  Connected: " + String(isWiFiConnected() ? "Yes" : "No"));
    Serial.println("  SSID: " + ssid);
    Serial.println("  IP: " + getLocalIP());
    Serial.println("  Signal: " + String(WiFi.RSSI()) + " dBm");
  }
}
