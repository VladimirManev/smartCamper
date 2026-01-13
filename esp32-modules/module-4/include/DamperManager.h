// Damper Manager
// Coordinates all damper controllers and handles MQTT commands

#ifndef DAMPER_MANAGER_H
#define DAMPER_MANAGER_H

#include "Config.h"
#include "ModuleManager.h"
#include "DamperController.h"

class DamperManager {
private:
  ModuleManager* moduleManager;
  DamperController** dampers;  // Array of pointers
  int numDampers;
  
  // Static pointer for MQTT callback
  static DamperManager* currentInstance;
  
  // Safety check: verifies that after changing a damper, there will be at least
  // 1 damper at 90° OR at least 2 dampers at 45° or more
  bool canChangeDamper(int index, int newAngle) const;

public:
  DamperManager(ModuleManager* moduleMgr);
  ~DamperManager();
  
  // Initialization
  void begin();
  
  // Main loop - call this in your main loop()
  void loop();
  
  // MQTT command handling
  void handleMQTTCommand(const String& commandJson);
  static void handleMQTTCommandStatic(const String& commandJson);
  
  // Force update - publish all damper statuses
  void forceUpdate();
  
  // Status
  void printStatus() const;
  
  // Getters
  DamperController* getDamper(int index) {
    if (index >= 0 && index < numDampers && dampers) {
      return dampers[index];
    }
    return nullptr;
  }
  
  // Safety check for external use (e.g., from DamperController)
  bool checkCanChangeDamper(int index, int newAngle) const {
    return canChangeDamper(index, newAngle);
  }
};

#endif

