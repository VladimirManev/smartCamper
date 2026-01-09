// Table Manager
// Coordinates table controller and handles MQTT commands

#ifndef TABLE_MANAGER_H
#define TABLE_MANAGER_H

#include "Config.h"
#include "ModuleManager.h"
#include "TableController.h"

class TableManager {
private:
  ModuleManager* moduleManager;
  TableController* tableController;
  
  // Static pointer for MQTT callback
  static TableManager* currentInstance;

public:
  TableManager(ModuleManager* moduleMgr);
  ~TableManager();
  
  // Initialization
  void begin();
  
  // Main loop - call this in your main loop()
  void loop();
  
  // MQTT command handling
  void handleMQTTCommand(const String& commandJson);
  static void handleMQTTCommandStatic(const String& commandJson);
  
  // Force update - publish current status
  void forceUpdate();
  
  // Status
  void printStatus() const;
  
  // Getters
  TableController* getTableController() { return tableController; }
};

#endif

