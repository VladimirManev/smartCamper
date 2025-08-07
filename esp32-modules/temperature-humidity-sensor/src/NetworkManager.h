#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"

class NetworkManager {
private:
    WiFiClient espClient;
    PubSubClient mqttClient;
    bool wifiConnected;
    bool mqttConnected;
    
public:
    NetworkManager();
    
    // WiFi функции
    bool connectWiFi();
    bool isWiFiConnected();
    void reconnectWiFi();
    
    // MQTT функции
    bool connectMQTT();
    bool isMQTTConnected();
    void reconnectMQTT();
    bool publishMessage(const char* topic, const char* message);
    void loop();
    
    // Общи функции
    void setup();
    void maintainConnections();
};

#endif 