// Network Manager Implementation
// Универсален WiFi мениджър за ESP32 модули

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
  // Не записваме SSID в flash (всеки boot е "чист")
  WiFi.persistent(false);
  
  // Изчистваме всички стари WiFi записи (неблокиращо - само веднъж)
  WiFi.disconnect(true, true);  // true,true = изчистване на flash
  // НЕ използваме delay() тук - неблокиращо
  
  // Задаваме режим и настройки
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  
  // Гарантираме, че използваме DHCP (не статичен IP)
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
  
  if (DEBUG_SERIAL) {
    Serial.println("🔌 Connecting to WiFi: " + ssid);
  }
  
  // Започваме свързване (неблокиращо)
  WiFi.begin(ssid.c_str(), password.length() > 0 ? password.c_str() : NULL);
  
  // НЕ чакаме тук - ще проверим статуса в loop()
  isConnected = false;
  lastReconnectAttempt = millis();
  
  if (DEBUG_SERIAL) {
    Serial.println("⏳ WiFi connection started, will check status in loop()");
  }
}

void NetworkManager::loop() {
  unsigned long currentTime = millis();
  
  // Активна проверка на WiFi връзката на интервали
  if (currentTime - lastWiFiCheck > WIFI_CHECK_INTERVAL) {
    lastWiFiCheck = currentTime;
    
    // Ако WiFi.status() показва свързано, но ping не работи, считаме че е мъртва връзка
    if (WiFi.status() == WL_CONNECTED) {
      if (!checkWiFiConnection()) {
        // Връзката е мъртва, форсираме реконекция
        if (DEBUG_SERIAL) {
          Serial.println("⚠️ WiFi connection is dead (no ping response), forcing reconnect");
        }
        isConnected = false;
        WiFi.disconnect();
        disconnectPending = true;  // Маркираме че трябва disconnect преди следващия опит
        lastReconnectAttempt = currentTime - WIFI_RECONNECT_DELAY; // Форсираме опит за реконекция
      } else {
        isConnected = true;
      }
    }
  }
  
  if (!isWiFiConnected()) {
    if (currentTime - lastReconnectAttempt > WIFI_RECONNECT_DELAY) {
      lastReconnectAttempt = currentTime;
      
      // Проверяваме дали WiFi се е свързал между опитите (от auto-reconnect)
      if (WiFi.status() == WL_CONNECTED) {
        isConnected = true;
        disconnectPending = false;  // Успешна конекция, не трябва disconnect
        if (DEBUG_SERIAL) {
          Serial.println("✅ WiFi connected (auto-reconnect)!");
          Serial.println("IP: " + getLocalIP());
        }
      } else {
        // Все още не сме свързани, правим нов опит
        disconnectPending = true;  // Маркираме че трябва disconnect преди следващия опит
        connect();  // Започваме опит (неблокиращо)
        
        // Проверяваме статуса веднага (без delay) - рядко ще е свързан веднага
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
      // Все още чакаме между опитите, но проверяваме дали случайно не се свързахме
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
    // Вече сме свързани, не трябва disconnect
    disconnectPending = false;
  }
}

bool NetworkManager::connect() {
  // Не записваме SSID в flash
  WiFi.persistent(false);
  
  // Изчистваме старите записи само ако е нужно (преди нов опит след неуспешен)
  if (disconnectPending) {
    WiFi.disconnect(true, true);
    disconnectPending = false;
    // Не използваме delay() - неблокиращо
  }
  
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
  
  if (DEBUG_SERIAL) {
    Serial.println("🔄 Attempting WiFi connection...");
  }
  
  // Започваме свързване (неблокиращо)
  WiFi.begin(ssid.c_str(), password.length() > 0 ? password.c_str() : NULL);
  
  // НЕ блокираме тук - просто започваме опита и ще проверим статуса в следващия loop()
  // Статусът ще се провери в loop() на следващата итерация
  
  return false;  // Все още не сме свързани, ще проверим в loop()
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
  // Проверяваме дали имаме gateway IP
  IPAddress gateway = WiFi.gatewayIP();
  if (gateway == INADDR_NONE || gateway[0] == 0) {
    return false;
  }
  
  // Проверяваме дали имаме валиден local IP
  IPAddress localIP = WiFi.localIP();
  if (localIP == INADDR_NONE || localIP[0] == 0) {
    return false;
  }
  
  // Допълнителна проверка - дали RSSI е разумен (не е -100 dBm)
  int rssi = WiFi.RSSI();
  if (rssi < -90) {
    // Сигналът е много слаб, може да има проблем
    if (DEBUG_SERIAL) {
      Serial.println("⚠️ WiFi RSSI is very weak: " + String(rssi) + " dBm");
    }
    // Не считаме за мъртва връзка, но е предупреждение
  }
  
  return true;
}

String NetworkManager::getLocalIP() {
  return WiFi.localIP().toString();
}

