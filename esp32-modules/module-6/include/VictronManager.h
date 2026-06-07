// Victron Manager
// BLE scanning, device cache, and MQTT status publishing for Victron devices

#ifndef VICTRON_MANAGER_H
#define VICTRON_MANAGER_H

#include "Config.h"
#include "ModuleManager.h"
#include "CommandHandler.h"
#include "VictronBleParser.h"

enum VictronDeviceRole {
  ROLE_SMARTSHUNT = 0,
  ROLE_ORION = 1,
  ROLE_MPPT1 = 2,
  ROLE_MPPT2 = 3,
  ROLE_AC_CHARGER = 4
};

class VictronManager {
 private:
  ModuleManager *moduleManager;
  CommandHandler commandHandler;

  unsigned long lastPublishMs;
  bool devicesConfigured;
  bool bleInitialized;
  bool bleScanActive;

  void startBle();

 public:
  VictronManager(ModuleManager *moduleMgr);

  void begin();
  void loop();
  void handleForceUpdate();

  CommandHandler &getCommandHandler() { return commandHandler; }

  void publishFullStatus();
  void printStatus() const;
};

#endif
