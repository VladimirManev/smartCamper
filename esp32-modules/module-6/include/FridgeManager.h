// Fridge BLE GATT client (Alpicool / AAOBOSI) + local mains power relay — module-6

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
  bool sessionBusy;
  unsigned long lastQueryMs;
  unsigned long lastReconnectAttemptMs;
  unsigned long reconnectIntervalMs;
  uint8_t connectFailStreak;
  unsigned long lastPublishMs;
  uint32_t statusNotifySeq;
  uint32_t statusNotifySeqAtQuery;

  uint8_t notifyBuf[64];
  size_t notifyLen;

  uint8_t lastStatus[64];
  size_t lastStatusLen;
  bool haveStatus;
  volatile bool statusDirty; // set in BLE notify; publish only from loop()
  bool powerRelayOn;         // local FRIDGE_RELAY_PIN
  bool powerOn;              // from fridge BLE status frame
  bool ecoMode;
  int8_t setpointC;
  int8_t setpoint2C;
  int8_t tempC;
  int8_t temp2C;
  bool haveZone2;

  bool openSession();
  void demoteLink(const char *reason);
  void noteConnectSuccess();
  void noteConnectFailure();
  void resetReconnectBackoff();
  bool ensureLinkForCommand();
  bool enableNotifications();
  bool writeFridge(const uint8_t *data, size_t len, bool quietRadio);
  bool sendBind();
  bool sendQuery(bool quietRadio = false);
  bool sendQueryAndWait(unsigned long waitMs, bool quietRadio = false);
  bool sendSetTemp(int8_t tempC, uint8_t zoneCmd);
  bool sendSettingsPatch(uint8_t settingsIndex, uint8_t value);
  void parseStatusFrame(const uint8_t *frame, size_t len);
  void setupButtons();
  void handleButtons();
  void applyPowerRelay(bool on, bool publish);
  void publishStatus();

 public:
  FridgeManager(ModuleManager *moduleMgr, VictronManager *victron);

  void begin();
  void loop();

  void onNotify(uint8_t *pData, size_t length);
  void handleForceUpdate();

  bool isPowerRelayOn() const { return powerRelayOn; }
  void setPower(bool on);
  void togglePower();

  // MQTT / button commands — reuse link or reconnect, then keep link for status
  bool setModeEco(bool eco);
  bool setModeToggle();
  bool setZoneTemp(uint8_t zone, int8_t temp);
};

#endif
