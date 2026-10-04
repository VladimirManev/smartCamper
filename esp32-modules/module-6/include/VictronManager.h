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
  unsigned long scanBlockedUntilMs;
  bool devicesConfigured;
  bool bleInitialized;
  bool bleScanActive;
  bool fridgeGattConnected;

  void startBle();

 public:
  VictronManager(ModuleManager *moduleMgr);

  void begin();
  void loop();
  void handleForceUpdate();

  CommandHandler &getCommandHandler() { return commandHandler; }

  void publishFullStatus();
  void printStatus() const;

  bool isBleReady() const { return bleInitialized; }
  void pauseScan(); // stop scan immediately
  /** Stop scan and keep it from restarting until millis() passes. */
  void blockScanFor(unsigned long durationMs);
  /** Fridge GATT up — brief quiet after connect; scan cadence stays normal. */
  void setFridgeGattConnected(bool connected);
  bool isFridgeGattConnected() const { return fridgeGattConnected; }
};

#endif
