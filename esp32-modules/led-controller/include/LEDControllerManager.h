// LED Controller Manager
// Coordinates WiFi, MQTT and LED control

#ifndef LED_CONTROLLER_MANAGER_H
#define LED_CONTROLLER_MANAGER_H

#include "Config.h"
#include "NetworkManager.h"
#include "MQTTManager.h"

// Forward declarations - variables and functions from main.cpp that we'll use
#include "StripState.h"  // Full definition needed for accessing members
extern bool relayStates[];
extern StripState stripStates[];

// Functions from main.cpp
extern void turnOnStrip(uint8_t stripIndex);
extern void turnOffStrip(uint8_t stripIndex);
extern void toggleStrip(uint8_t stripIndex);
extern void setBrightnessSmooth(uint8_t stripIndex, uint8_t targetBrightness);
extern void setStripMode(uint8_t stripIndex, StripMode mode);
extern void toggleRelay(uint8_t relayIndex = 0);
extern bool isAnyButtonPressed();

class LEDControllerManager {
private:
  NetworkManager networkManager;
  MQTTManager mqttManager;
  
  unsigned long lastStatusPublish;
  unsigned long lastHeartbeat;
  bool mqttInitialized;
  
  // Static pointer for MQTT callback
  static LEDControllerManager* currentInstance;
  
  // MQTT callback handler
  static void handleMQTTMessage(char* topic, byte* payload, unsigned int length);
  void processMQTTCommand(char* topic, byte* payload, unsigned int length);
  
  // Publish full status (all strips + relays)
  void publishFullStatus();

public:
  LEDControllerManager();
  
  void begin();
  void loop();
  
  // Publish status data
  void publishStatus();
  void publishStripStatus(uint8_t stripIndex);
  void publishRelayStatus();
  
  // Connection checks
  bool isWiFiConnected();
  bool isMQTTConnected();
  
  void printStatus();
};

#endif

