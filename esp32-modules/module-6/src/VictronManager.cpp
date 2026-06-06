// Victron Manager Implementation

#include "VictronManager.h"
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <ArduinoJson.h>
#include <cstring>

#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
#define USE_String
#endif

static const size_t VICTRON_MANUFACTURER_DATA_MAX = 31;

struct VictronDeviceConfig {
  const char *name;
  const char *mac;
  uint8_t key[16];
  VictronDeviceRole role;
  uint8_t expectedRecordType;
  uint16_t lastDataCounter;
  bool configured;
};

struct CachedSmartShunt {
  bool hasData;
  SmartShuntReading reading;
  unsigned long updatedAt;
};

struct CachedMppt {
  bool hasData;
  MpptReading reading;
  unsigned long updatedAt;
};

struct CachedOrion {
  bool hasData;
  OrionReading reading;
  unsigned long updatedAt;
};

static VictronDeviceConfig devices[VICTRON_DEVICE_COUNT];
static CachedSmartShunt smartShuntCache = {false, {}, 0};
static CachedMppt mppt1Cache = {false, {}, 0};
static CachedMppt mppt2Cache = {false, {}, 0};
static CachedOrion orionCache = {false, {}, 0};
static BLEScan *bleScan = nullptr;

static bool copyManufacturerData(BLEAdvertisedDevice &device, uint8_t *out, size_t *outLen,
                                 size_t maxLen) {
#ifdef USE_String
  String manufacturerData = device.getManufacturerData();
  if (manufacturerData.length() == 0 || manufacturerData.length() > maxLen) {
    return false;
  }
  memcpy(out, manufacturerData.c_str(), manufacturerData.length());
  *outLen = manufacturerData.length();
#else
  std::string manufacturerData = device.getManufacturerData();
  if (manufacturerData.length() == 0 || manufacturerData.length() > maxLen) {
    return false;
  }
  memcpy(out, manufacturerData.data(), manufacturerData.length());
  *outLen = manufacturerData.length();
#endif
  return true;
}

static VictronDeviceConfig *findDevice(const BLEAddress &address) {
  for (size_t i = 0; i < VICTRON_DEVICE_COUNT; i++) {
    if (devices[i].configured && address == BLEAddress(devices[i].mac)) {
      return &devices[i];
    }
  }
  return nullptr;
}

static void updateSmartShuntCache(const SmartShuntReading &reading, unsigned long nowMs) {
  smartShuntCache.hasData = true;
  smartShuntCache.reading = reading;
  smartShuntCache.updatedAt = nowMs;
}

static void updateMpptCache(VictronDeviceRole role, const MpptReading &reading, unsigned long nowMs) {
  CachedMppt *cache = role == ROLE_MPPT1 ? &mppt1Cache : &mppt2Cache;
  cache->hasData = true;
  cache->reading = reading;
  cache->updatedAt = nowMs;
}

static void updateOrionCache(const OrionReading &reading, unsigned long nowMs) {
  orionCache.hasData = true;
  orionCache.reading = reading;
  orionCache.updatedAt = nowMs;
}

static bool handleParsedPayload(VictronDeviceConfig *device, uint8_t recordType,
                                const uint8_t *plain, size_t plainLen, unsigned long nowMs) {
  switch (device->role) {
    case ROLE_SMARTSHUNT:
      if (recordType != RECORD_BATTERY_MONITOR) {
        return false;
      }
      {
        SmartShuntReading reading;
        if (!parseBatteryMonitor(plain, plainLen, reading)) {
          return false;
        }
        updateSmartShuntCache(reading, nowMs);
      }
      return true;

    case ROLE_MPPT1:
    case ROLE_MPPT2:
      if (recordType != RECORD_SOLAR_CHARGER) {
        return false;
      }
      {
        MpptReading reading;
        if (!parseSolarCharger(plain, plainLen, reading)) {
          return false;
        }
        updateMpptCache(device->role, reading, nowMs);
      }
      return true;

    case ROLE_ORION:
      if (recordType != RECORD_ORION_XS) {
        return false;
      }
      {
        OrionReading reading;
        if (!parseOrionXs(plain, plainLen, reading)) {
          return false;
        }
        updateOrionCache(reading, nowMs);
      }
      return true;

    default:
      return false;
  }
}

class VictronScanCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) override {
    VictronDeviceConfig *device = findDevice(advertisedDevice.getAddress());
    if (device == nullptr) {
      return;
    }

    uint8_t manufacturerData[VICTRON_MANUFACTURER_DATA_MAX];
    size_t manufacturerLen = 0;
    if (!copyManufacturerData(advertisedDevice, manufacturerData, &manufacturerLen,
                            sizeof(manufacturerData))) {
      return;
    }

    if (manufacturerLen < 10) {
      return;
    }

    uint16_t vendorId = manufacturerData[0] | ((uint16_t)manufacturerData[1] << 8);
    if (vendorId != VICTRON_VENDOR_ID) {
      return;
    }

    const uint8_t *record = manufacturerData + 2;
    size_t recordLen = manufacturerLen - 2;

    if (recordLen < 8 || record[0] != VICTRON_BEACON_TYPE) {
      return;
    }

    if (record[7] != device->key[0]) {
      return;
    }

    uint8_t recordType = record[4];
    if (!isSupportedRecordType(recordType)) {
      return;
    }

    uint16_t dataCounter = record[5] | ((uint16_t)record[6] << 8);
    if (dataCounter == device->lastDataCounter) {
      return;
    }

    const uint8_t *cipher = record + 8;
    size_t cipherLen = recordLen - 8;
    if (cipherLen == 0 || cipherLen > 16) {
      return;
    }

    uint8_t plain[16] = {0};
    if (!decryptVictronPayload(device->key, cipher, cipherLen, record[5], record[6], plain)) {
      return;
    }

    unsigned long nowMs = millis();
    if (!handleParsedPayload(device, recordType, plain, cipherLen, nowMs)) {
      return;
    }

    device->lastDataCounter = dataCounter;

    if (DEBUG_VERBOSE && DEBUG_SERIAL) {
      Serial.print("Updated ");
      Serial.println(device->name);
    }
  }
};

static bool initDevices() {
  struct DeviceEntry {
    const char *name;
    const char *mac;
    const char *key;
    VictronDeviceRole role;
  };

  const DeviceEntry entries[VICTRON_DEVICE_COUNT] = {
      {VICTRON_DEVICE_0_NAME, VICTRON_DEVICE_0_MAC, VICTRON_DEVICE_0_KEY, ROLE_SMARTSHUNT},
      {VICTRON_DEVICE_1_NAME, VICTRON_DEVICE_1_MAC, VICTRON_DEVICE_1_KEY, ROLE_ORION},
      {VICTRON_DEVICE_2_NAME, VICTRON_DEVICE_2_MAC, VICTRON_DEVICE_2_KEY, ROLE_MPPT1},
      {VICTRON_DEVICE_3_NAME, VICTRON_DEVICE_3_MAC, VICTRON_DEVICE_3_KEY, ROLE_MPPT2},
  };

  for (size_t i = 0; i < VICTRON_DEVICE_COUNT; i++) {
    devices[i].name = entries[i].name;
    devices[i].mac = entries[i].mac;
    devices[i].role = entries[i].role;
    devices[i].expectedRecordType = 0;
    devices[i].lastDataCounter = 0xFFFF;
    devices[i].configured = parseHexKey(entries[i].key, devices[i].key);

    if (!devices[i].configured) {
      if (DEBUG_SERIAL) {
        Serial.print("ERROR: Invalid key for ");
        Serial.println(entries[i].name);
      }
      return false;
    }

    if (DEBUG_SERIAL) {
      Serial.print("Configured Victron device: ");
      Serial.print(entries[i].name);
      Serial.print(" (");
      Serial.print(entries[i].mac);
      Serial.println(")");
    }
  }

  return true;
}

static void appendSmartShuntJson(JsonDocument &doc) {
  if (!smartShuntCache.hasData) {
    doc["smartshunt"] = nullptr;
    return;
  }

  JsonObject obj = doc.createNestedObject("smartshunt");
  const SmartShuntReading &r = smartShuntCache.reading;

  if (r.voltageValid) {
    obj["voltage"] = roundTo1Decimal(r.voltage);
  }
  if (r.currentValid) {
    obj["current"] = roundTo2Decimals(r.current);
  }
  if (r.socValid) {
    obj["soc"] = r.soc;
  }
  if (r.consumedAhValid) {
    obj["consumedAh"] = roundTo1Decimal(r.consumedAh);
  }
  if (r.timeToGoValid) {
    obj["timeToGoMin"] = r.timeToGoMin;
  } else {
    obj["timeToGoMin"] = nullptr;
  }
  obj["alarmReason"] = r.alarmReason;
  obj["updatedAt"] = smartShuntCache.updatedAt;
}

static void appendMpptJson(JsonDocument &doc, const char *key, const CachedMppt &cache) {
  if (!cache.hasData) {
    doc[key] = nullptr;
    return;
  }

  JsonObject obj = doc.createNestedObject(key);
  const MpptReading &r = cache.reading;

  obj["deviceState"] = r.deviceState;
  obj["errorCode"] = r.errorCode;
  if (r.batteryVoltageValid) {
    obj["batteryVoltage"] = roundTo1Decimal(r.batteryVoltage);
  }
  if (r.batteryCurrentValid) {
    obj["batteryCurrent"] = roundTo2Decimals(r.batteryCurrent);
  }
  if (r.pvPowerValid) {
    obj["pvPower"] = r.pvPower;
  }
  if (r.yieldTodayValid) {
    obj["yieldTodayKwh"] = roundTo2Decimals(r.yieldTodayKwh);
  }
  obj["updatedAt"] = cache.updatedAt;
}

static void appendOrionJson(JsonDocument &doc) {
  if (!orionCache.hasData) {
    doc["orion"] = nullptr;
    return;
  }

  JsonObject obj = doc.createNestedObject("orion");
  const OrionReading &r = orionCache.reading;

  obj["deviceState"] = r.deviceState;
  obj["errorCode"] = r.errorCode;
  if (r.outputVoltageValid) {
    obj["outputVoltage"] = roundTo1Decimal(r.outputVoltage);
  }
  if (r.outputCurrentValid) {
    obj["outputCurrent"] = roundTo2Decimals(r.outputCurrent);
  }
  if (r.inputVoltageValid) {
    obj["inputVoltage"] = roundTo1Decimal(r.inputVoltage);
  }
  if (r.inputCurrentValid) {
    obj["inputCurrent"] = roundTo2Decimals(r.inputCurrent);
  }
  obj["offReason"] = r.offReason;
  obj["updatedAt"] = orionCache.updatedAt;
}

VictronManager::VictronManager(ModuleManager *moduleMgr)
    : commandHandler(&moduleMgr->getMQTTManager(), this, MODULE_ID),
      lastPublishMs(0),
      bleScanActive(false) {
  moduleManager = moduleMgr;
}

void VictronManager::begin() {
  if (!initDevices()) {
    if (DEBUG_SERIAL) {
      Serial.println("ERROR: Victron device initialization failed");
    }
    return;
  }

  BLEDevice::init("");
  bleScan = BLEDevice::getScan();
  bleScan->setAdvertisedDeviceCallbacks(new VictronScanCallbacks());
  bleScan->setActiveScan(true);
  bleScan->setInterval(BLE_SCAN_INTERVAL_MS);
  bleScan->setWindow(BLE_SCAN_WINDOW_MS);
  bleScan->start(0, false);
  bleScanActive = true;

  if (DEBUG_SERIAL) {
    Serial.println("Victron BLE continuous scan started");
    Serial.println("Instant Readout via Bluetooth must be enabled on each device");
  }
}

void VictronManager::loop() {
  if (moduleManager == nullptr) {
    return;
  }

  unsigned long nowMs = millis();
  if (nowMs - lastPublishMs >= VICTRON_STATUS_PUBLISH_INTERVAL_MS) {
    publishFullStatus();
    lastPublishMs = nowMs;
  }
}

void VictronManager::handleForceUpdate() {
  publishFullStatus();
  lastPublishMs = millis();
}

void VictronManager::publishFullStatus() {
  if (!moduleManager || !moduleManager->isConnected()) {
    return;
  }

  StaticJsonDocument<1024> doc;
  doc["publishedAt"] = millis();

  appendSmartShuntJson(doc);
  appendMpptJson(doc, "mppt1", mppt1Cache);
  appendMpptJson(doc, "mppt2", mppt2Cache);
  appendOrionJson(doc);

#if AC_CHARGER_ENABLED
  doc["acCharger"] = nullptr;
#else
  doc["acCharger"] = nullptr;
#endif

  String jsonString;
  serializeJson(doc, jsonString);

  String topic = String(MQTT_TOPIC_SENSORS) + MODULE_ID + "/status";
  moduleManager->getMQTTManager().publishRaw(topic, jsonString);

  if (DEBUG_VERBOSE && DEBUG_MQTT) {
    Serial.println("Published Victron status: " + jsonString);
  }
}

void VictronManager::printStatus() const {
  if (!DEBUG_SERIAL) {
    return;
  }

  Serial.println("Victron Manager Status:");
  Serial.println("  BLE scan active: " + String(bleScanActive ? "Yes" : "No"));
  Serial.println("  SmartShunt data: " + String(smartShuntCache.hasData ? "Yes" : "No"));
  Serial.println("  MPPT1 data: " + String(mppt1Cache.hasData ? "Yes" : "No"));
  Serial.println("  MPPT2 data: " + String(mppt2Cache.hasData ? "Yes" : "No"));
  Serial.println("  Orion data: " + String(orionCache.hasData ? "Yes" : "No"));
}
