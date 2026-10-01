// Fridge BLE GATT client (Alpicool / AAOBOSI) for combined Victron+fridge test

#ifndef FRIDGE_MANAGER_H
#define FRIDGE_MANAGER_H

#include "Config.h"
#include "ModuleManager.h"
#include "VictronManager.h"
#include <ArduinoJson.h>

class FridgeManager {
 private:
  ModuleManager *moduleManager;
  VictronManager *victronManager;

  bool connected;
  unsigned long lastQueryMs;
  unsigned long lastReconnectAttemptMs;
  unsigned long lastPublishMs;

  uint8_t notifyBuf[64];
  size_t notifyLen;

  uint8_t lastStatus[64];
  size_t lastStatusLen;
  bool haveStatus;
  bool powerOn;
  bool ecoMode;
  int8_t setpointC;
  int8_t setpoint2C;
  int8_t tempC;
  int8_t temp2C;
  bool haveZone2;

  bool tryConnect();
  bool enableNotifications();
  bool writeFridge(const uint8_t *data, size_t len);
  bool sendBind();
  bool sendQuery();
  bool sendSetTemp(int8_t tempC, uint8_t zoneCmd);
  bool sendSettingsPatch(uint8_t settingsIndex, uint8_t value);
  void parseStatusFrame(const uint8_t *frame, size_t len);
  void setupButtons();
  void handleButtons();
  void publishStatus();

 public:
  FridgeManager(ModuleManager *moduleMgr, VictronManager *victron);

  void begin();
  void loop();

  void onNotify(uint8_t *pData, size_t length);
  void handleForceUpdate();

  // MQTT commands
  bool setModeEco(bool eco);
  bool setModeToggle();
  bool setZoneTemp(uint8_t zone, int8_t temp);
};

#endif
