// Gray Water Level Manager
// Specific logic for gray water level sensor

#ifndef WATER_LEVEL_MANAGER_H
#define WATER_LEVEL_MANAGER_H

#include "Config.h"
#include "NetworkManager.h"
#include "MQTTManager.h"
#include "CommandHandler.h"

class WaterLevelManager {
private:
  NetworkManager networkManager;
  MQTTManager mqttManager;
  CommandHandler commandHandler;
  
  // GPIO pins array (from bottom to top)
  int levelPins[NUM_LEVEL_PINS];
  int levelPercentages[NUM_LEVEL_PINS];
  
  // Timing
  unsigned long lastSensorRead;
  unsigned long lastDataSent;
  unsigned long lastStatusLog;
  
  // Measurement data
  float measurements[5];  // Store last 5 measurements for averaging
  int measurementIndex;
  int measurementCount;
  
  // Last published value
  float lastPublishedLevel;
  bool forceUpdateRequested;
  
  // Sensor functions
  int readWaterLevel();  // Returns level index (0-7) or -1 if error
  float levelToPercent(int level);  // Converts level index to percentage
  void setupPins();
  void setPinsLow();  // Set all pins to LOW after measurement
  
public:
  WaterLevelManager();
  
  void begin();
  void loop();
  
  // Force update function (public for CommandHandler)
  void handleForceUpdate();
  
  // MQTT callback (static for MQTTManager)
  static void handleMQTTMessage(char* topic, byte* payload, unsigned int length);
  
  void printStatus();

private:
  // Static pointer to current instance
  static WaterLevelManager* currentInstance;
};

#endif

