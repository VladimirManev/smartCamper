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
  // Не записваме SSID в flash (всеки boot е "чист")
  WiFi.persistent(false);
  
  // Изчистваме всички стари WiFi записи
  WiFi.disconnect(true, true);  // true,true = изчистване на flash
  delay(500);
  
  // Задаваме режим и настройки
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  
  // Гарантираме, че използваме DHCP (не статичен IP)
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
  
  if (DEBUG_SERIAL) {
    Serial.println("🔍 ESP32 MAC Address: " + WiFi.macAddress());
    Serial.println("🔍 WiFi Mode: " + String(WiFi.getMode()));
    Serial.println("🔍 Scanning for networks...");
    int networksFound = WiFi.scanNetworks();
    Serial.println("Found " + String(networksFound) + " networks:");
    
    for (int i = 0; i < networksFound; i++) {
      String networkName = WiFi.SSID(i);
      int signalStrength = WiFi.RSSI(i);
      Serial.println("  " + String(i + 1) + ". " + networkName + " (Signal: " + String(signalStrength) + " dBm)");
      
      if (networkName == ssid) {
        Serial.println("    ✅ Target network found!");
      }
    }
    
    Serial.println("🔌 Connecting to WiFi...");
    Serial.println("SSID: " + ssid);
    Serial.println("Password: " + password);
  }
  
  // Започваме свързване
  WiFi.begin(ssid.c_str(), password.length() > 0 ? password.c_str() : NULL);
  
  // Правим първи опит за свързване (чакаме до 10 секунди)
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
    // Задаваме lastReconnectAttempt, за да може loop() да опита отново след WIFI_RECONNECT_DELAY
    lastReconnectAttempt = millis();
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
  // Не записваме SSID в flash
  WiFi.persistent(false);
  
  // Изчистваме старите записи преди всеки опит
  WiFi.disconnect(true, true);
  delay(500);
  
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
  
  if (DEBUG_SERIAL) {
    Serial.println("🔄 Attempting WiFi connection...");
    Serial.println("SSID: " + ssid);
    Serial.println("Password: " + password);
  }
  
  // Започваме свързване
  WiFi.begin(ssid.c_str(), password.length() > 0 ? password.c_str() : NULL);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {  // Увеличаваме до 30 опита (15 секунди)
    delay(500);
    attempts++;
    if (DEBUG_SERIAL) {
      Serial.print(".");
      if (attempts % 10 == 0) {
        Serial.println();
        Serial.println("WiFi Status: " + String(WiFi.status()));
        Serial.println("Local IP: " + WiFi.localIP().toString());
        Serial.println("Gateway IP: " + WiFi.gatewayIP().toString());
      }
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
  return WiFi.status() == WL_CONNECTED;
}

String NetworkManager::getLocalIP() {
  return WiFi.localIP().toString();
}
