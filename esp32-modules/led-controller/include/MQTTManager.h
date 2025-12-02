// MQTT Manager
// Универсален MQTT мениджър за ESP32 модули

#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <PubSubClient.h>
#include <WiFi.h>
#include "Config.h"

class MQTTManager {
private:
  WiFiClient wifiClient;
  PubSubClient mqttClient;
  String clientId;
  String brokerIP;
  int brokerPort;
  unsigned long lastReconnectAttempt;
  bool isConnected;
  int failedAttempts;  // Брояч на неуспешни опити
  unsigned long lastWiFiWarningTime;  // Последно време на предупреждение за WiFi

public:
  MQTTManager();
  MQTTManager(String clientId, String brokerIP, int brokerPort);
  
  void begin();
  void loop();
  void loop(bool wifiConnected);  // Overload с WiFi статус
  bool connect();
  void disconnect();
  bool isMQTTConnected();
  int getFailedAttempts();
  
  // Публикуване на данни
  bool publishSensorData(String sensorType, String value);
  bool publishSensorData(String sensorType, float value);
  bool publishSensorData(String sensorType, int value);
  
  // Абониране за команди
  bool subscribeToCommands(String moduleType);
  
  // Callback за получени съобщения
  void setCallback(void (*callback)(char* topic, byte* payload, unsigned int length));
  
  void printStatus();
};

#endif

