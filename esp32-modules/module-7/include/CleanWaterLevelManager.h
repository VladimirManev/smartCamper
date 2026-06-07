// Clean Water Level Manager
// Coordinates sensor and command handling for module-7

#ifndef CLEAN_WATER_LEVEL_MANAGER_H
#define CLEAN_WATER_LEVEL_MANAGER_H

#include "Config.h"
#include "ModuleManager.h"
#include "CommandHandler.h"
#include "CleanWaterLevelSensor.h"

class CleanWaterLevelManager {
private:
  ModuleManager* moduleManager;
  CleanWaterLevelSensor cleanWaterLevelSensor;
  CommandHandler commandHandler;

public:
  CleanWaterLevelManager(ModuleManager* moduleMgr);

  void begin();
  void loop();
  void handleForceUpdate();

  CommandHandler& getCommandHandler() { return commandHandler; }

  void printStatus() const;
};

#endif
