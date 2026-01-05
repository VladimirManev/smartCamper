// Network Manager
// Universal WiFi manager for ESP32 modules

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
  
  // Active WiFi connection check
  bool checkWiFiConnection();

public:
  NetworkManager();
  NetworkManager(String ssid, String password);
  
  void begin();
  void loop();
  bool connect();
  void disconnect();
  bool isWiFiConnected() const;
  String getLocalIP() const;
};

#endif
