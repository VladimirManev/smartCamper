// LED Controller Manager
// Координира WiFi, MQTT и LED управлението

#ifndef LED_CONTROLLER_MANAGER_H
#define LED_CONTROLLER_MANAGER_H

#include "Config.h"
#include "NetworkManager.h"
#include "MQTTManager.h"

// Forward declarations - променливите и функциите от main.cpp които ще използваме
#include "StripState.h"  // Full definition needed for accessing members
extern bool relayState;
extern StripState stripStates[];

// Functions from main.cpp
extern void turnOnStrip(uint8_t stripIndex);
extern void turnOffStrip(uint8_t stripIndex);
extern void toggleStrip(uint8_t stripIndex);
extern void setBrightnessSmooth(uint8_t stripIndex, uint8_t targetBrightness);
extern void toggleRelay();
extern bool isAnyButtonPressed();

class LEDControllerManager {
private:
  NetworkManager networkManager;
  MQTTManager mqttManager;
  
  unsigned long lastStatusPublish;
  unsigned long lastHeartbeat;
  bool mqttInitialized;
  
  // Статичен указател за MQTT callback
  static LEDControllerManager* currentInstance;
  
  // MQTT callback handler
  static void handleMQTTMessage(char* topic, byte* payload, unsigned int length);
  void processMQTTCommand(char* topic, byte* payload, unsigned int length);

public:
  LEDControllerManager();
  
  void begin();
  void loop();
  
  // Публикуване на статус данни
  void publishStatus();
  void publishStripStatus(uint8_t stripIndex);
  void publishRelayStatus();
  void publishHeartbeat();  // Heartbeat - модулът е жив
  
  // Проверки за връзка
  bool isWiFiConnected();
  bool isMQTTConnected();
  
  void printStatus();
};

#endif

