// Fridge BLE GATT client — Arduino BLE, shares radio with Victron scan
//
// Local mains relay (GPIO FRIDGE_RELAY_PIN) + BLE GATT while power is ON.
// Never background-connect while relay is OFF — BLEClient::connect() blocks
// ~30s when the fridge is gone and kills WiFi/MQTT + Victron scan.

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

static BtnState gBtnRelay = {FRIDGE_RELAY_BTN_PIN, false, false, 0};
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
      sessionBusy(false),
      lastQueryMs(0),
      lastReconnectAttemptMs(0),
      reconnectIntervalMs(FRIDGE_RECONNECT_INTERVAL_MS),
      connectFailStreak(0),
      lastPublishMs(0),
      statusNotifySeq(0),
      statusNotifySeqAtQuery(0),
      notifyLen(0),
      lastStatusLen(0),
      haveStatus(false),
      statusDirty(false),
      powerRelayOn(false),
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
  pinMode(FRIDGE_RELAY_PIN, OUTPUT);
  digitalWrite(FRIDGE_RELAY_PIN, LOW);
  powerRelayOn = false;

  setupButtons();
  if (DEBUG_SERIAL) {
    Serial.println("FridgeManager ready (local power relay + BLE)");
    Serial.printf("  MAC %s  relay GPIO %d  btn GPIO %d\n", FRIDGE_BLE_MAC,
                  FRIDGE_RELAY_PIN, FRIDGE_RELAY_BTN_PIN);
  }
  publishStatus();
}

void FridgeManager::applyPowerRelay(bool on, bool publish) {
  if (powerRelayOn == on) {
    if (publish) {
      statusDirty = true;
    }
    return;
  }

  powerRelayOn = on;
  digitalWrite(FRIDGE_RELAY_PIN, on ? HIGH : LOW);
  Serial.printf("fridge mains relay %s (GPIO %d)\n", on ? "ON" : "OFF",
                FRIDGE_RELAY_PIN);

  if (!on) {
    if (connected) {
      demoteLink("power relay OFF");
    }
    resetReconnectBackoff();
  } else {
    resetReconnectBackoff();
    lastReconnectAttemptMs = millis() - FRIDGE_RECONNECT_INTERVAL_MS;
  }

  if (publish) {
    statusDirty = true;
  }
}

void FridgeManager::setPower(bool on) {
  applyPowerRelay(on, true);
}

void FridgeManager::togglePower() {
  setPower(!powerRelayOn);
}

void FridgeManager::resetReconnectBackoff() {
  reconnectIntervalMs = FRIDGE_RECONNECT_INTERVAL_MS;
  connectFailStreak = 0;
}

void FridgeManager::noteConnectSuccess() {
  resetReconnectBackoff();
}

void FridgeManager::noteConnectFailure() {
  // Count from *now* — connect() itself may have blocked for many seconds.
  lastReconnectAttemptMs = millis();
  if (connectFailStreak < 255) {
    connectFailStreak++;
  }
  if (connectFailStreak >= FRIDGE_RECONNECT_FAIL_FAST) {
    reconnectIntervalMs = FRIDGE_RECONNECT_MAX_MS;
  } else {
    const unsigned long next = reconnectIntervalMs * 2UL;
    reconnectIntervalMs =
        next > FRIDGE_RECONNECT_MAX_MS ? FRIDGE_RECONNECT_MAX_MS : next;
  }
  if (victronManager != nullptr) {
    victronManager->clearScanBlock();
  }
  Serial.printf("fridge reconnect backoff now %lu ms (fails=%u)\n",
                reconnectIntervalMs, static_cast<unsigned>(connectFailStreak));
}

void FridgeManager::setupButtons() {
  pinMode(FRIDGE_RELAY_BTN_PIN, INPUT_PULLUP);
  pinMode(FRIDGE_BTN_POWER, INPUT_PULLUP);
  pinMode(FRIDGE_BTN_Z1_UP, INPUT_PULLUP);
  pinMode(FRIDGE_BTN_Z1_DOWN, INPUT_PULLUP);
  pinMode(FRIDGE_BTN_Z2_UP, INPUT_PULLUP);
  pinMode(FRIDGE_BTN_Z2_DOWN, INPUT_PULLUP);

  BtnState *btns[] = {&gBtnRelay, &gBtnPower, &gBtnUp, &gBtnDown, &gBtnUp2,
                      &gBtnDown2};
  for (BtnState *b : btns) {
    b->lastRaw = digitalRead(b->pin) == LOW;
    b->stablePressed = b->lastRaw;
  }
  Serial.printf("Fridge buttons: relay=%d BLE-PWR=%d Z1=%d/%d Z2=%d/%d\n",
                FRIDGE_RELAY_BTN_PIN, FRIDGE_BTN_POWER, FRIDGE_BTN_Z1_UP,
                FRIDGE_BTN_Z1_DOWN, FRIDGE_BTN_Z2_UP, FRIDGE_BTN_Z2_DOWN);
}

void FridgeManager::publishStatus() {
  if (moduleManager == nullptr || !moduleManager->isConnected()) {
    return;
  }

  StaticJsonDocument<256> doc;
  doc["connected"] = connected && haveStatus;
  doc["powerRelay"] = powerRelayOn ? "ON" : "OFF";
  doc["mode"] = ecoMode ? "eco" : "max";
  doc["updatedAt"] = millis();
  doc["haveStatus"] = haveStatus;

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
    z2["temp"] = setpoint2C;
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

  statusNotifySeq++;
  // Never MQTT-publish from the BLE notify callback — that tears down PubSubClient/TCP.
  statusDirty = true;
}

void FridgeManager::demoteLink(const char *reason) {
  Serial.printf("fridge link demoted: %s\n",
                reason != nullptr ? reason : "unknown");
  connected = false;
  gWriteChar = nullptr;
  gNotifyChar = nullptr;
  notifyLen = 0;
  if (gClient != nullptr && gClient->isConnected()) {
    gClient->disconnect();
    delay(150);
  }
  if (victronManager != nullptr) {
    victronManager->setFridgeGattConnected(false);
  }
  statusDirty = true;
}

bool FridgeManager::writeFridge(const uint8_t *data, size_t len, bool quietRadio) {
  if (gWriteChar == nullptr || len == 0 || !connected) {
    return false;
  }
  if (quietRadio && victronManager != nullptr) {
    victronManager->blockScanFor(FRIDGE_CMD_RADIO_QUIET_MS);
    delay(40);
  }

  // Alpicool protocol: ATT Write Request (with response). Write-without-response
  // is accepted by the stack but the fridge often drops set/mode frames.
  bool withResponse = true;
  if (gWriteChar->canWrite()) {
    withResponse = true;
  } else if (gWriteChar->canWriteNoResponse()) {
    withResponse = false;
    Serial.println("fridge write: falling back to no-response");
  } else {
    return false;
  }

  size_t offset = 0;
  while (offset < len) {
    const size_t chunk = (len - offset > 20) ? 20 : (len - offset);
    gWriteChar->writeValue(const_cast<uint8_t *>(data + offset), chunk,
                           withResponse);
    offset += chunk;
    // Separate Write Requests per chunk (app does the same).
    delay(withResponse ? 20 : 5);
  }

  if (gClient == nullptr || !gClient->isConnected()) {
    demoteLink("disconnect during write");
    return false;
  }
  return true;
}

bool FridgeManager::sendQuery(bool quietRadio) {
  if (!connected || gWriteChar == nullptr) {
    return false;
  }
  static const uint8_t query[] = {0xFE, 0xFE, 0x03, 0x01, 0x02, 0x00};
  notifyLen = 0;
  statusNotifySeqAtQuery = statusNotifySeq;
  return writeFridge(query, sizeof(query), quietRadio);
}

bool FridgeManager::sendQueryAndWait(unsigned long waitMs, bool quietRadio) {
  if (!sendQuery(quietRadio)) {
    return false;
  }
  const unsigned long start = millis();
  while (millis() - start < waitMs) {
    delay(10);
    if (statusNotifySeq != statusNotifySeqAtQuery) {
      // Safe: we are on the main/MQTT task, not inside the BLE notify callback.
      if (statusDirty) {
        statusDirty = false;
        publishStatus();
      }
      return true;
    }
  }
  return false;
}

bool FridgeManager::sendBind() {
  static const uint8_t bindFrame[] = {0xFE, 0xFE, 0x03, 0x00, 0x01, 0xFF};
  return writeFridge(bindFrame, sizeof(bindFrame), true);
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
  uint8_t frame[7] = {0xFE, 0xFE, 0x04, zoneCmd, static_cast<uint8_t>(value), 0,
                      0};
  const uint16_t sum = checksum16(frame, 5);
  frame[5] = static_cast<uint8_t>(sum >> 8);
  frame[6] = static_cast<uint8_t>(sum & 0xFF);
  if (DEBUG_SERIAL) {
    Serial.printf("fridge TX set z%u %dC\n", zoneCmd == 0x06 ? 2 : 1,
                  static_cast<int>(value));
  }
  return writeFridge(frame, sizeof(frame), true);
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

  return writeFridge(frame, sizeof(frame), true);
}

bool FridgeManager::enableNotifications() {
  if (gNotifyChar == nullptr) {
    return false;
  }
  BLERemoteDescriptor *cccd =
      gNotifyChar->getDescriptor(BLEUUID((uint16_t)0x2902));
  if (cccd != nullptr) {
    uint8_t enable[] = {0x01, 0x00};
    cccd->writeValue(enable, 2, true);
  }
  gNotifyChar->registerForNotify(notifyCallback);
  return true;
}

bool FridgeManager::openSession() {
  if (victronManager == nullptr || !victronManager->isBleReady()) {
    return false;
  }

  Serial.printf("fridge connecting %s...\n", FRIDGE_BLE_MAC);
  // Short quiet only — do not hold Victron scan for the whole connect timeout.
  victronManager->blockScanFor(FRIDGE_CMD_RADIO_QUIET_MS);
  delay(80);

  if (gClient == nullptr) {
    gClient = BLEDevice::createClient();
  }
  // Note: this Arduino-ESP32 BLEClient has no setConnectTimeout(); connect() may
  // block ~30s when the fridge is powered off — backoff + clearScanBlock mitigate.
  if (gClient->isConnected()) {
    gClient->disconnect();
    delay(200);
  }

  BLEAddress addr(FRIDGE_BLE_MAC);
  if (!gClient->connect(addr)) {
    Serial.println("fridge connect failed");
    connected = false;
    victronManager->setFridgeGattConnected(false);
    noteConnectFailure();
    return false;
  }

  BLERemoteService *service = gClient->getService(kServiceUUID);
  if (service == nullptr) {
    Serial.println("fridge service 1234 missing");
    gClient->disconnect();
    victronManager->setFridgeGattConnected(false);
    noteConnectFailure();
    return false;
  }

  gWriteChar = service->getCharacteristic(kWriteUUID);
  gNotifyChar = service->getCharacteristic(kNotifyUUID);
  if (gWriteChar == nullptr || gNotifyChar == nullptr ||
      !gNotifyChar->canNotify()) {
    Serial.println("fridge chars missing");
    gClient->disconnect();
    victronManager->setFridgeGattConnected(false);
    noteConnectFailure();
    return false;
  }

  if (!enableNotifications()) {
    gClient->disconnect();
    victronManager->setFridgeGattConnected(false);
    noteConnectFailure();
    return false;
  }

  delay(300);
  connected = true;
  victronManager->setFridgeGattConnected(true);
  // Official app: enable notify → query (no bind). Wait for status before any set.
  if (!sendQueryAndWait(FRIDGE_QUERY_RESPONSE_MS, true)) {
    Serial.println("fridge connected but status query failed");
    // Still mark linked — caller may retry query; avoid instant demote loop.
  }
  lastQueryMs = millis();
  noteConnectSuccess();
  Serial.println("fridge connected");
  return true;
}

bool FridgeManager::ensureLinkForCommand() {
  // User-initiated — try soon even if background backoff is long (fridge was OFF).
  resetReconnectBackoff();

  if (gClient != nullptr && gClient->isConnected() && connected &&
      gWriteChar != nullptr) {
    // Probe write path — silent death otherwise looks connected forever.
    if (sendQueryAndWait(FRIDGE_QUERY_RESPONSE_MS, true)) {
      return true;
    }
    Serial.println("fridge ensureLink: probe failed — reconnect on command");
    demoteLink("cmd probe");
  } else if (connected) {
    demoteLink("stale flags");
  }

  if (!openSession()) {
    return false;
  }
  // openSession already waited for a status frame when possible.
  if (!haveStatus && !sendQueryAndWait(FRIDGE_QUERY_RESPONSE_MS, true)) {
    Serial.println("fridge ensureLink: no status after connect");
    return false;
  }
  return true;
}

bool FridgeManager::setModeEco(bool eco) {
  if (sessionBusy) {
    Serial.println("fridge mode: session busy");
    return false;
  }
  sessionBusy = true;
  bool ok = false;
  if (ensureLinkForCommand()) {
    if (!haveStatus &&
        !sendQueryAndWait(FRIDGE_QUERY_RESPONSE_MS, true)) {
      Serial.println("fridge mode: no status for settings patch");
    } else if (sendSettingsPatch(2, eco ? 0x01 : 0x00)) {
      delay(300);
      ok = sendQueryAndWait(FRIDGE_QUERY_RESPONSE_MS, true) && ecoMode == eco;
      if (!ok) {
        Serial.printf("fridge mode not confirmed (want %s got %s)\n",
                      eco ? "eco" : "max", ecoMode ? "eco" : "max");
        demoteLink("mode unconfirmed");
        if (openSession() && sendQueryAndWait(FRIDGE_QUERY_RESPONSE_MS, true) &&
            sendSettingsPatch(2, eco ? 0x01 : 0x00)) {
          delay(300);
          ok = sendQueryAndWait(FRIDGE_QUERY_RESPONSE_MS, true) &&
               ecoMode == eco;
        }
      }
    } else {
      Serial.println("fridge mode: settings patch write failed");
    }
  } else {
    Serial.println("fridge mode: no link");
  }
  // Keep link on success for status; demote only on hard failure.
  if (!ok && connected) {
    // Leave link up if GATT still works — status can continue.
  }
  sessionBusy = false;
  lastQueryMs = millis();
  return ok;
}

bool FridgeManager::setModeToggle() {
  return setModeEco(!ecoMode);
}

bool FridgeManager::setZoneTemp(uint8_t zone, int8_t temp) {
  if (zone != 1 && zone != 2) {
    return false;
  }
  if (sessionBusy) {
    Serial.println("fridge setTemp: session busy");
    return false;
  }
  const uint8_t zoneCmd = (zone == 1) ? 0x05 : 0x06;

  sessionBusy = true;
  bool ok = false;
  if (ensureLinkForCommand()) {
    Serial.printf("fridge setTemp z%u → %dC\n", zone, static_cast<int>(temp));
    if (sendSetTemp(temp, zoneCmd)) {
      delay(300);
      if (sendQueryAndWait(FRIDGE_QUERY_RESPONSE_MS, true)) {
        const int8_t got = (zone == 1) ? setpointC : setpoint2C;
        ok = got == temp;
        if (!ok) {
          Serial.printf("fridge set not confirmed z%u want %d got %d\n", zone,
                        static_cast<int>(temp), static_cast<int>(got));
        }
      } else {
        Serial.println("fridge setTemp: no status after write");
      }
    } else {
      Serial.println("fridge setTemp: write failed");
    }
    if (!ok) {
      demoteLink("setTemp unconfirmed");
      if (openSession() && sendSetTemp(temp, zoneCmd)) {
        delay(300);
        if (sendQueryAndWait(FRIDGE_QUERY_RESPONSE_MS, true)) {
          const int8_t got = (zone == 1) ? setpointC : setpoint2C;
          ok = got == temp;
        }
      }
    }
  } else {
    Serial.println("fridge setTemp: no link");
  }
  sessionBusy = false;
  lastQueryMs = millis();
  return ok;
}

void FridgeManager::handleButtons() {
  if (pollButton(gBtnRelay)) {
    togglePower();
  }
  if (pollButton(gBtnPower)) {
    if (sessionBusy) {
      return;
    }
    sessionBusy = true;
    if (ensureLinkForCommand()) {
      if (!haveStatus) {
        sendQueryAndWait(FRIDGE_QUERY_RESPONSE_MS, true);
      }
      sendSettingsPatch(1, powerOn ? 0x00 : 0x01);
      delay(200);
      sendQueryAndWait(FRIDGE_QUERY_RESPONSE_MS, true);
    }
    sessionBusy = false;
    lastQueryMs = millis();
  }
  if (pollButton(gBtnUp)) {
    setZoneTemp(1, static_cast<int8_t>(setpointC + 1));
  }
  if (pollButton(gBtnDown)) {
    setZoneTemp(1, static_cast<int8_t>(setpointC - 1));
  }
  if (pollButton(gBtnUp2)) {
    setZoneTemp(2, static_cast<int8_t>(setpoint2C + 1));
  }
  if (pollButton(gBtnDown2)) {
    setZoneTemp(2, static_cast<int8_t>(setpoint2C - 1));
  }
}

void FridgeManager::loop() {
#if !FRIDGE_ENABLED
  return;
#endif
  handleButtons();

  if (statusDirty) {
    statusDirty = false;
    publishStatus();
  }

  if (sessionBusy) {
    return;
  }

  if (!connected) {
    // Do not BLE-connect while mains relay is OFF (protect WiFi/MQTT/Victron).
    if (!powerRelayOn) {
      return;
    }
    if (victronManager != nullptr && victronManager->isBleReady() &&
        millis() - lastReconnectAttemptMs >= reconnectIntervalMs) {
      openSession();
      if (connected) {
        lastReconnectAttemptMs = millis();
      }
    }
    return;
  }

  if (gClient == nullptr || !gClient->isConnected()) {
    demoteLink("GATT disconnect");
    return;
  }

  if (millis() - lastQueryMs >= FRIDGE_QUERY_INTERVAL_MS) {
    sendQuery(false);
    lastQueryMs = millis();
  }
}
