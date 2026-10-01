// Fridge BLE GATT client — Arduino BLE, shares radio with Victron scan

#include "FridgeManager.h"

#include <BLEDevice.h>
#include <BLEClient.h>
#include <BLERemoteCharacteristic.h>
#include <BLEUtils.h>
#include <cstring>

static BLEUUID kServiceUUID("00001234-0000-1000-8000-00805f9b34fb");
static BLEUUID kWriteUUID("00001235-0000-1000-8000-00805f9b34fb");
static BLEUUID kNotifyUUID("00001236-0000-1000-8000-00805f9b34fb");

static BLEClient *gClient = nullptr;
static BLERemoteCharacteristic *gWriteChar = nullptr;
static BLERemoteCharacteristic *gNotifyChar = nullptr;
static FridgeManager *gFridgeInstance = nullptr;

static uint16_t checksum16(const uint8_t *frame, size_t lenWithoutChecksum) {
  uint16_t sum = 0;
  for (size_t i = 0; i < lenWithoutChecksum; i++) {
    sum = static_cast<uint16_t>(sum + frame[i]);
  }
  return sum;
}

static void notifyCallback(BLERemoteCharacteristic * /*pChar*/, uint8_t *pData,
                           size_t length, bool /*isNotify*/) {
  if (gFridgeInstance != nullptr) {
    gFridgeInstance->onNotify(pData, length);
  }
}

struct BtnState {
  int pin;
  bool stablePressed;
  bool lastRaw;
  unsigned long lastChangeMs;
};

static BtnState gBtnPower = {FRIDGE_BTN_POWER, false, false, 0};
static BtnState gBtnUp = {FRIDGE_BTN_Z1_UP, false, false, 0};
static BtnState gBtnDown = {FRIDGE_BTN_Z1_DOWN, false, false, 0};
static BtnState gBtnUp2 = {FRIDGE_BTN_Z2_UP, false, false, 0};
static BtnState gBtnDown2 = {FRIDGE_BTN_Z2_DOWN, false, false, 0};

static bool pollButton(BtnState &btn) {
  const bool rawPressed = digitalRead(btn.pin) == LOW;
  const unsigned long now = millis();
  if (rawPressed != btn.lastRaw) {
    btn.lastRaw = rawPressed;
    btn.lastChangeMs = now;
  }
  if ((now - btn.lastChangeMs) < FRIDGE_BTN_DEBOUNCE_MS) {
    return false;
  }
  if (rawPressed == btn.stablePressed) {
    return false;
  }
  btn.stablePressed = rawPressed;
  return rawPressed;
}

FridgeManager::FridgeManager(ModuleManager *moduleMgr, VictronManager *victron)
    : moduleManager(moduleMgr),
      victronManager(victron),
      connected(false),
      lastQueryMs(0),
      lastReconnectAttemptMs(0),
      lastPublishMs(0),
      notifyLen(0),
      lastStatusLen(0),
      haveStatus(false),
      powerOn(false),
      ecoMode(false),
      setpointC(0),
      setpoint2C(0),
      tempC(0),
      temp2C(0),
      haveZone2(false) {
  gFridgeInstance = this;
}

void FridgeManager::begin() {
#if !FRIDGE_ENABLED
  return;
#endif
  setupButtons();
  if (DEBUG_SERIAL) {
    Serial.println("FridgeManager ready (connects after Victron BLE init)");
    Serial.printf("  MAC %s\n", FRIDGE_BLE_MAC);
  }
}

void FridgeManager::setupButtons() {
  pinMode(FRIDGE_BTN_POWER, INPUT_PULLUP);
  pinMode(FRIDGE_BTN_Z1_UP, INPUT_PULLUP);
  pinMode(FRIDGE_BTN_Z1_DOWN, INPUT_PULLUP);
  pinMode(FRIDGE_BTN_Z2_UP, INPUT_PULLUP);
  pinMode(FRIDGE_BTN_Z2_DOWN, INPUT_PULLUP);

  BtnState *btns[] = {&gBtnPower, &gBtnUp, &gBtnDown, &gBtnUp2, &gBtnDown2};
  for (BtnState *b : btns) {
    b->lastRaw = digitalRead(b->pin) == LOW;
    b->stablePressed = b->lastRaw;
  }
  Serial.println("Fridge buttons: 32=PWR 33/25=Z1 26/27=Z2 (to GND)");
}

void FridgeManager::publishStatus() {
  if (moduleManager == nullptr || !moduleManager->isConnected()) {
    return;
  }

  StaticJsonDocument<256> doc;
  doc["connected"] = connected && haveStatus;
  doc["mode"] = ecoMode ? "eco" : "max";
  doc["updatedAt"] = millis();

  JsonObject z1 = doc.createNestedObject("zone1");
  if (haveStatus) {
    z1["temp"] = tempC;
    z1["setpoint"] = setpointC;
  } else {
    z1["temp"] = nullptr;
    z1["setpoint"] = nullptr;
  }

  JsonObject z2 = doc.createNestedObject("zone2");
  if (haveStatus && haveZone2) {
    z2["temp"] = temp2C;
    z2["setpoint"] = setpoint2C;
  } else if (haveStatus) {
    z2["temp"] = setpoint2C; // still publish setpoint if present
    z2["setpoint"] = setpoint2C;
  } else {
    z2["temp"] = nullptr;
    z2["setpoint"] = nullptr;
  }

  String json;
  serializeJson(doc, json);
  String topic = String(MQTT_TOPIC_SENSORS) + MODULE_ID + "/fridge";
  moduleManager->getMQTTManager().publishRaw(topic, json);
  lastPublishMs = millis();
}

void FridgeManager::handleForceUpdate() {
  publishStatus();
}

void FridgeManager::onNotify(uint8_t *pData, size_t length) {
  if (length == 0 || length > sizeof(notifyBuf)) {
    return;
  }

  if (notifyLen == 0) {
    if (length >= 2 && pData[0] == 0xFE && pData[1] == 0xFE) {
      memcpy(notifyBuf, pData, length);
      notifyLen = length;
    } else {
      return;
    }
  } else {
    if (notifyLen + length > sizeof(notifyBuf)) {
      notifyLen = 0;
      return;
    }
    memcpy(notifyBuf + notifyLen, pData, length);
    notifyLen += length;
  }

  if (notifyLen < 3) {
    return;
  }
  const size_t expected = static_cast<size_t>(notifyBuf[2]) + 3;
  if (notifyLen < expected) {
    return;
  }
  parseStatusFrame(notifyBuf, expected);
  notifyLen = 0;
}

void FridgeManager::parseStatusFrame(const uint8_t *frame, size_t len) {
  if (len < 6 || frame[0] != 0xFE || frame[1] != 0xFE || frame[3] != 0x01) {
    return;
  }
  const size_t expected = static_cast<size_t>(frame[2]) + 3;
  if (len < expected) {
    return;
  }

  if (expected > 5) {
    powerOn = frame[5] != 0;
  }
  if (expected > 6) {
    ecoMode = frame[6] != 0;
  }
  if (expected > 8) {
    setpointC = static_cast<int8_t>(frame[8]);
  }
  if (expected > 18) {
    tempC = static_cast<int8_t>(frame[18]);
  }
  if (expected > 22) {
    setpoint2C = static_cast<int8_t>(frame[22]);
  }
  if (expected > 30 && frame[30] != 0x80 && frame[30] != 0xFD) {
    haveZone2 = true;
    temp2C = static_cast<int8_t>(frame[30]);
  } else {
    haveZone2 = false;
  }

  if (DEBUG_SERIAL) {
    Serial.printf("fridge power=%s mode=%s set=%dC temp=%dC set2=%dC",
                  powerOn ? "ON" : "OFF", ecoMode ? "ECO" : "MAX",
                  static_cast<int>(setpointC), static_cast<int>(tempC),
                  static_cast<int>(setpoint2C));
    if (haveZone2) {
      Serial.printf(" temp2=%dC", static_cast<int>(temp2C));
    }
    Serial.println();
  }

  if (expected <= sizeof(lastStatus)) {
    memcpy(lastStatus, frame, expected);
    lastStatusLen = expected;
    haveStatus = true;
  }

  publishStatus();
}

bool FridgeManager::writeFridge(const uint8_t *data, size_t len) {
  if (gWriteChar == nullptr || len == 0) {
    return false;
  }
  const bool withResponse = gWriteChar->canWrite();
  size_t offset = 0;
  while (offset < len) {
    const size_t chunk = (len - offset > 20) ? 20 : (len - offset);
    gWriteChar->writeValue(const_cast<uint8_t *>(data + offset), chunk, withResponse);
    offset += chunk;
    delay(5);
  }
  return true;
}

bool FridgeManager::sendQuery() {
  if (!connected || gWriteChar == nullptr) {
    return false;
  }
  static const uint8_t query[] = {0xFE, 0xFE, 0x03, 0x01, 0x02, 0x00};
  notifyLen = 0;
  return writeFridge(query, sizeof(query));
}

bool FridgeManager::sendBind() {
  static const uint8_t bindFrame[] = {0xFE, 0xFE, 0x03, 0x00, 0x01, 0xFF};
  return writeFridge(bindFrame, sizeof(bindFrame));
}

bool FridgeManager::sendSetTemp(int8_t value, uint8_t zoneCmd) {
  if (!connected) {
    return false;
  }
  if (value < FRIDGE_TEMP_MIN_C) {
    value = FRIDGE_TEMP_MIN_C;
  }
  if (value > FRIDGE_TEMP_MAX_C) {
    value = FRIDGE_TEMP_MAX_C;
  }
  uint8_t frame[7] = {0xFE, 0xFE, 0x04, zoneCmd, static_cast<uint8_t>(value), 0, 0};
  const uint16_t sum = checksum16(frame, 5);
  frame[5] = static_cast<uint8_t>(sum >> 8);
  frame[6] = static_cast<uint8_t>(sum & 0xFF);
  if (DEBUG_SERIAL) {
    Serial.printf("fridge TX set z%u %dC\n", zoneCmd == 0x06 ? 2 : 1,
                  static_cast<int>(value));
  }
  return writeFridge(frame, sizeof(frame));
}

bool FridgeManager::sendSettingsPatch(uint8_t settingsIndex, uint8_t value) {
  if (!connected || !haveStatus || lastStatusLen < 34) {
    Serial.println("fridge: no status for settings patch");
    return false;
  }
  if (settingsIndex >= 25) {
    return false;
  }

  uint8_t settings[25];
  memcpy(settings, lastStatus + 4, 14);
  memcpy(settings + 14, lastStatus + 22, 8);
  settings[22] = lastStatus[33];
  settings[23] = lastStatus[32];
  settings[24] = 0x00;
  settings[settingsIndex] = value;

  uint8_t frame[31];
  frame[0] = 0xFE;
  frame[1] = 0xFE;
  frame[2] = 0x1C;
  frame[3] = 0x02;
  memcpy(frame + 4, settings, 25);
  const uint16_t sum = checksum16(frame, 29);
  frame[29] = static_cast<uint8_t>(sum >> 8);
  frame[30] = static_cast<uint8_t>(sum & 0xFF);

  if (!writeFridge(frame, sizeof(frame))) {
    return false;
  }
  delay(200);
  return sendQuery();
}

bool FridgeManager::setModeEco(bool eco) {
  return sendSettingsPatch(2, eco ? 0x01 : 0x00);
}

bool FridgeManager::setModeToggle() {
  return setModeEco(!ecoMode);
}

bool FridgeManager::setZoneTemp(uint8_t zone, int8_t temp) {
  if (zone == 1) {
    return sendSetTemp(temp, 0x05);
  }
  if (zone == 2) {
    return sendSetTemp(temp, 0x06);
  }
  return false;
}

bool FridgeManager::enableNotifications() {
  if (gNotifyChar == nullptr) {
    return false;
  }
  BLERemoteDescriptor *cccd = gNotifyChar->getDescriptor(BLEUUID((uint16_t)0x2902));
  if (cccd != nullptr) {
    uint8_t enable[] = {0x01, 0x00};
    cccd->writeValue(enable, 2, true);
  }
  gNotifyChar->registerForNotify(notifyCallback);
  return true;
}

bool FridgeManager::tryConnect() {
  if (victronManager == nullptr || !victronManager->isBleReady()) {
    return false;
  }

  Serial.printf("fridge connecting %s...\n", FRIDGE_BLE_MAC);
  victronManager->pauseScan();
  delay(100);

  if (gClient == nullptr) {
    gClient = BLEDevice::createClient();
  }
  if (gClient->isConnected()) {
    gClient->disconnect();
    delay(300);
  }

  BLEAddress addr(FRIDGE_BLE_MAC);
  if (!gClient->connect(addr)) {
    Serial.println("fridge connect failed");
    connected = false;
    publishStatus();
    return false;
  }

  BLERemoteService *service = gClient->getService(kServiceUUID);
  if (service == nullptr) {
    Serial.println("fridge service 1234 missing");
    gClient->disconnect();
    return false;
  }

  gWriteChar = service->getCharacteristic(kWriteUUID);
  gNotifyChar = service->getCharacteristic(kNotifyUUID);
  if (gWriteChar == nullptr || gNotifyChar == nullptr || !gNotifyChar->canNotify()) {
    Serial.println("fridge chars missing");
    gClient->disconnect();
    return false;
  }

  if (!enableNotifications()) {
    gClient->disconnect();
    return false;
  }

  delay(200);
  connected = true;
  sendBind();
  delay(100);
  sendQuery();
  lastQueryMs = millis();
  Serial.println("fridge connected");
  return true;
}

void FridgeManager::handleButtons() {
  if (!connected) {
    return;
  }
  if (pollButton(gBtnPower)) {
    // Local test only — UI power stays on module-5 relay
    sendSettingsPatch(1, powerOn ? 0x00 : 0x01);
  }
  if (pollButton(gBtnUp)) {
    if (sendSetTemp(static_cast<int8_t>(setpointC + 1), 0x05)) {
      delay(150);
      sendQuery();
    }
  }
  if (pollButton(gBtnDown)) {
    if (sendSetTemp(static_cast<int8_t>(setpointC - 1), 0x05)) {
      delay(150);
      sendQuery();
    }
  }
  if (pollButton(gBtnUp2)) {
    if (sendSetTemp(static_cast<int8_t>(setpoint2C + 1), 0x06)) {
      delay(150);
      sendQuery();
    }
  }
  if (pollButton(gBtnDown2)) {
    if (sendSetTemp(static_cast<int8_t>(setpoint2C - 1), 0x06)) {
      delay(150);
      sendQuery();
    }
  }
}

void FridgeManager::loop() {
#if !FRIDGE_ENABLED
  return;
#endif
  handleButtons();

  if (connected) {
    if (gClient == nullptr || !gClient->isConnected()) {
      Serial.println("fridge link lost");
      connected = false;
      gWriteChar = nullptr;
      gNotifyChar = nullptr;
      publishStatus();
      return;
    }
    if (millis() - lastQueryMs >= FRIDGE_QUERY_INTERVAL_MS) {
      if (sendQuery()) {
        lastQueryMs = millis();
      } else {
        connected = false;
        publishStatus();
      }
    }
    return;
  }

  if (victronManager != nullptr && victronManager->isBleReady() &&
      millis() - lastReconnectAttemptMs >= FRIDGE_RECONNECT_INTERVAL_MS) {
    lastReconnectAttemptMs = millis();
    tryConnect();
  }
}
