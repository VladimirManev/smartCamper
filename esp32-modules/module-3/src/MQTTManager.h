// MQTT Manager
// Universal MQTT manager for ESP32 modules

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
  int failedAttempts;  // Counter for failed attempts
  unsigned long lastWiFiWarningTime;  // Last time of WiFi warning

public:
  MQTTManager();
  MQTTManager(String clientId, String brokerIP, int brokerPort);
  
  void begin();
  void loop();
  void loop(bool wifiConnected);  // Overload with WiFi status
  bool connect();
  void disconnect();
  bool isMQTTConnected();  // Cannot be const - PubSubClient methods are not const
  int getFailedAttempts() const;
  
  // Publish data
  bool publishSensorData(String sensorType, String value);
  bool publishSensorData(String sensorType, float value);
  bool publishSensorData(String sensorType, int value);
  
  // Publish raw topic (for heartbeat, etc.)
  bool publishRaw(String topic, String payload);
  
  // Subscribe to commands
  bool subscribeToCommands(String moduleType);
  
  // Callback for received messages
  void setCallback(void (*callback)(char* topic, byte* payload, unsigned int length));
  
  void printStatus();  // Cannot be const - PubSubClient methods are not const
};

#endif
