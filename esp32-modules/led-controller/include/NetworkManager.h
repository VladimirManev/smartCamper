// Network Manager
// Универсален WiFi мениджър за ESP32 модули

#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <WiFi.h>
#include "Config.h"

class NetworkManager {
private:
  String ssid;
  String password;
  unsigned long lastReconnectAttempt;
  unsigned long lastWiFiCheck;
  bool isConnected;
  
  // Активна проверка на WiFi връзката
  bool checkWiFiConnection();

public:
  NetworkManager();
  NetworkManager(String ssid, String password);
  
  void begin();
  void loop();
  bool connect();
  void disconnect();
  bool isWiFiConnected();
  String getLocalIP();
};

#endif

