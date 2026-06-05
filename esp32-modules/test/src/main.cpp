/**
 * Test module - read Victron BLE advertisements and print to Serial.
 */

#include <Arduino.h>
#include <cstring>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include "mbedtls/aes.h"
#include "Config.h"

#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
#define USE_String
#endif

static const uint16_t VICTRON_VENDOR_ID = 0x02E1;
static const uint8_t VICTRON_BEACON_TYPE = 0x10;
static const uint8_t RECORD_BATTERY_MONITOR = 0x02;
static const uint8_t RECORD_SOLAR_CHARGER = 0x01;
static const uint8_t RECORD_DCDC_CONVERTER = 0x04;
static const uint8_t RECORD_ORION_XS = 0x0F;

struct VictronDevice {
  const char *name;
  const char *mac;
  uint8_t key[16];
  uint16_t lastDataCounter;
  bool configured;
};

static VictronDevice devices[VICTRON_DEVICE_COUNT];
static BLEScan *bleScan = nullptr;

static void printHex(const uint8_t *data, size_t len) {
  for (size_t i = 0; i < len; i++) {
    if (data[i] < 0x10) {
      Serial.print('0');
    }
    Serial.print(data[i], HEX);
    if (i + 1 < len) {
      Serial.print(' ');
    }
  }
}

static bool parseHexKey(const char *hexKey, uint8_t *outKey) {
  if (strlen(hexKey) != 32) {
    return false;
  }

  for (size_t i = 0; i < 16; i++) {
    char byteStr[3] = {hexKey[i * 2], hexKey[i * 2 + 1], '\0'};
    outKey[i] = (uint8_t)strtoul(byteStr, nullptr, 16);
  }
  return true;
}

static const size_t VICTRON_MANUFACTURER_DATA_MAX = 31;

static bool copyManufacturerData(BLEAdvertisedDevice &device, uint8_t *out,
                                 size_t *outLen, size_t maxLen) {
#ifdef USE_String
  String manufacturerData = device.getManufacturerData();
  if (manufacturerData.length() == 0) {
    return false;
  }
  if (manufacturerData.length() > maxLen) {
    return false;
  }
  memcpy(out, manufacturerData.c_str(), manufacturerData.length());
  *outLen = manufacturerData.length();
#else
  std::string manufacturerData = device.getManufacturerData();
  if (manufacturerData.length() == 0) {
    return false;
  }
  if (manufacturerData.length() > maxLen) {
    return false;
  }
  memcpy(out, manufacturerData.data(), manufacturerData.length());
  *outLen = manufacturerData.length();
#endif
  return true;
}

static bool isSupportedRecordType(uint8_t recordType) {
  return recordType == RECORD_BATTERY_MONITOR || recordType == RECORD_SOLAR_CHARGER ||
         recordType == RECORD_DCDC_CONVERTER || recordType == RECORD_ORION_XS;
}

static bool decryptPayload(const uint8_t *key, const uint8_t *cipher, size_t cipherLen,
                           uint8_t counterLsb, uint8_t counterMsb,
                           uint8_t *plainOut) {
  if (cipherLen == 0 || cipherLen > 16) {
    return false;
  }

  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);

  if (mbedtls_aes_setkey_enc(&aes, key, 128) != 0) {
    mbedtls_aes_free(&aes);
    return false;
  }

  uint8_t nonce[16] = {counterLsb, counterMsb, 0};
  size_t ncOffset = 0;
  uint8_t streamBlock[16];

  int result = mbedtls_aes_crypt_ctr(&aes, cipherLen, &ncOffset, nonce,
                                     streamBlock, cipher, plainOut);
  mbedtls_aes_free(&aes);
  return result == 0;
}

class BitReader {
 public:
  BitReader(const uint8_t *data, size_t dataLen) : data(data), dataLen(dataLen), cursor(0) {}

  bool readBit() {
    if (cursor >= dataLen * 8) {
      return false;
    }
    size_t byteIndex = cursor / 8;
    uint8_t bitIndex = cursor % 8;
    bool value = (data[byteIndex] >> bitIndex) & 0x01;
    cursor++;
    return value;
  }

  uint32_t readUnsigned(uint8_t bitCount) {
    uint32_t value = 0;
    for (uint8_t i = 0; i < bitCount; i++) {
      if (readBit()) {
        value |= (1U << i);
      }
    }
    return value;
  }

  int32_t readSigned(uint8_t bitCount) {
    int32_t value = 0;
    for (uint8_t i = 0; i < bitCount - 1; i++) {
      if (readBit()) {
        value |= (1 << i);
      }
    }
    if (readBit()) {
      value -= (1 << (bitCount - 1));
    }
    return value;
  }

  void skip(uint8_t bitCount) {
    for (uint8_t i = 0; i < bitCount; i++) {
      readBit();
    }
  }

 private:
  const uint8_t *data;
  size_t dataLen;
  size_t cursor;
};

static void printOptionalFloat(const char *label, bool valid, float value, uint8_t decimals) {
  Serial.print(label);
  if (!valid) {
    Serial.println("N/A");
    return;
  }
  Serial.println(value, decimals);
}

static void printBatteryMonitorValues(const uint8_t *payload, size_t payloadLen) {
  BitReader reader(payload, payloadLen);

  uint16_t timeToGo = (uint16_t)reader.readUnsigned(16);
  int16_t voltageRaw = (int16_t)reader.readSigned(16);
  uint16_t alarmReason = (uint16_t)reader.readUnsigned(16);

  BitReader auxReader(payload, payloadLen);
  auxReader.skip(48);
  reader.skip(16);

  uint8_t auxInputType = (uint8_t)reader.readUnsigned(2);
  uint32_t currentRawU = reader.readUnsigned(22);
  uint32_t consumedRaw = reader.readUnsigned(20);
  uint16_t socRaw = (uint16_t)reader.readUnsigned(10);

  printOptionalFloat("  time_to_go_min: ", timeToGo != 0xFFFF, timeToGo, 0);
  printOptionalFloat("  voltage_V: ", voltageRaw != 0x7FFF, voltageRaw * 0.01f, 2);

  Serial.print("  current_A: ");
  if (currentRawU == 0x3FFFFF) {
    Serial.println("N/A");
  } else {
    int32_t currentRaw = (int32_t)currentRawU;
    if (currentRawU & (1U << 21)) {
      currentRaw = (int32_t)(currentRawU | (~0U << 22));
    }
    Serial.println(currentRaw * 0.001f, 3);
  }

  printOptionalFloat("  soc_percent: ", socRaw != 0x3FF, socRaw * 0.1f, 1);
  printOptionalFloat("  consumed_Ah: ", consumedRaw != 0xFFFFF, consumedRaw * -0.1f, 1);

  Serial.print("  alarm_reason: 0x");
  Serial.println(alarmReason, HEX);
  Serial.print("  aux_input_type: ");
  Serial.println(auxInputType);

  if (auxInputType == 0) {
    printOptionalFloat("  aux_voltage_V: ", true, auxReader.readSigned(16) * 0.01f, 2);
  } else if (auxInputType == 1) {
    printOptionalFloat("  mid_voltage_V: ", true, auxReader.readUnsigned(16) * 0.01f, 2);
  } else if (auxInputType == 2) {
    printOptionalFloat("  temperature_K: ", true, auxReader.readUnsigned(16) * 0.01f, 2);
  }
}

static void printSolarChargerValues(const uint8_t *payload, size_t payloadLen) {
  BitReader reader(payload, payloadLen);

  uint8_t deviceState = (uint8_t)reader.readUnsigned(8);
  uint8_t errorCode = (uint8_t)reader.readUnsigned(8);
  int16_t batteryVoltageRaw = (int16_t)reader.readSigned(16);
  int16_t batteryCurrentRaw = (int16_t)reader.readSigned(16);
  uint16_t yieldTodayRaw = (uint16_t)reader.readUnsigned(16);
  uint16_t pvPowerRaw = (uint16_t)reader.readUnsigned(16);
  uint16_t loadCurrentRaw = (uint16_t)reader.readUnsigned(9);

  Serial.print("  device_state: ");
  Serial.println(deviceState);
  Serial.print("  error_code: ");
  Serial.println(errorCode);
  printOptionalFloat("  battery_voltage_V: ", batteryVoltageRaw != 0x7FFF, batteryVoltageRaw * 0.01f, 2);
  printOptionalFloat("  battery_current_A: ", batteryCurrentRaw != 0x7FFF, batteryCurrentRaw * 0.1f, 1);
  printOptionalFloat("  yield_today_kWh: ", yieldTodayRaw != 0xFFFF, yieldTodayRaw * 0.01f, 2);
  printOptionalFloat("  pv_power_W: ", pvPowerRaw != 0xFFFF, pvPowerRaw, 0);
  printOptionalFloat("  load_current_A: ", loadCurrentRaw != 0x1FF, loadCurrentRaw * 0.1f, 1);
}

static void printDcdcConverterValues(const uint8_t *payload, size_t payloadLen) {
  BitReader reader(payload, payloadLen);

  uint8_t deviceState = (uint8_t)reader.readUnsigned(8);
  uint8_t errorCode = (uint8_t)reader.readUnsigned(8);
  uint16_t inputVoltageRaw = (uint16_t)reader.readUnsigned(16);
  int16_t outputVoltageRaw = (int16_t)reader.readSigned(16);
  uint32_t offReason = reader.readUnsigned(32);

  Serial.print("  device_state: ");
  Serial.println(deviceState);
  Serial.print("  error_code: ");
  Serial.println(errorCode);
  printOptionalFloat("  input_voltage_V: ", inputVoltageRaw != 0xFFFF, inputVoltageRaw * 0.01f, 2);
  printOptionalFloat("  output_voltage_V: ", outputVoltageRaw != 0x7FFF, outputVoltageRaw * 0.01f, 2);
  Serial.print("  off_reason: 0x");
  Serial.println(offReason, HEX);
}

static void printOrionXsValues(const uint8_t *payload, size_t payloadLen) {
  BitReader reader(payload, payloadLen);

  uint8_t deviceState = (uint8_t)reader.readUnsigned(8);
  uint8_t errorCode = (uint8_t)reader.readUnsigned(8);
  int16_t outputVoltageRaw = (int16_t)reader.readSigned(16);
  int16_t outputCurrentRaw = (int16_t)reader.readSigned(16);
  uint16_t inputVoltageRaw = (uint16_t)reader.readUnsigned(16);
  uint16_t inputCurrentRaw = (uint16_t)reader.readUnsigned(16);
  uint32_t offReason = reader.readUnsigned(32);

  Serial.print("  device_state: ");
  Serial.println(deviceState);
  Serial.print("  error_code: ");
  Serial.println(errorCode);
  printOptionalFloat("  output_voltage_V: ", outputVoltageRaw != 0x7FFF, outputVoltageRaw * 0.01f, 2);
  printOptionalFloat("  output_current_A: ", outputCurrentRaw != 0x7FFF, outputCurrentRaw * 0.1f, 1);
  printOptionalFloat("  input_voltage_V: ", inputVoltageRaw != 0xFFFF, inputVoltageRaw * 0.01f, 2);
  printOptionalFloat("  input_current_A: ", inputCurrentRaw != 0xFFFF, inputCurrentRaw * 0.1f, 1);
  Serial.print("  off_reason: 0x");
  Serial.println(offReason, HEX);
}

static void printParsedValues(uint8_t recordType, const uint8_t *payload, size_t payloadLen) {
  switch (recordType) {
    case RECORD_BATTERY_MONITOR:
      printBatteryMonitorValues(payload, payloadLen);
      break;
    case RECORD_SOLAR_CHARGER:
      printSolarChargerValues(payload, payloadLen);
      break;
    case RECORD_DCDC_CONVERTER:
      printDcdcConverterValues(payload, payloadLen);
      break;
    case RECORD_ORION_XS:
      printOrionXsValues(payload, payloadLen);
      break;
    default:
      Serial.print("  Unsupported record type: 0x");
      Serial.println(recordType, HEX);
      break;
  }
}

static VictronDevice *findDevice(const BLEAddress &address) {
  for (size_t i = 0; i < VICTRON_DEVICE_COUNT; i++) {
    if (devices[i].configured && address == BLEAddress(devices[i].mac)) {
      return &devices[i];
    }
  }
  return nullptr;
}

class VictronScanCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) override {
    VictronDevice *device = findDevice(advertisedDevice.getAddress());
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
    if (!decryptPayload(device->key, cipher, cipherLen, record[5], record[6], plain)) {
      return;
    }

    device->lastDataCounter = dataCounter;

    Serial.print("--- ");
    Serial.print(device->name);
    Serial.println(" ---");
    Serial.print("Name: ");
    Serial.println(advertisedDevice.getName().c_str());
    Serial.print("RSSI: ");
    Serial.println(advertisedDevice.getRSSI());
    Serial.print("Record type: 0x");
    Serial.println(recordType, HEX);
    Serial.print("Counter: ");
    Serial.println(dataCounter);

    Serial.print("RAW manufacturer data: ");
    printHex(manufacturerData, manufacturerLen);
    Serial.println();

    Serial.print("Decrypted payload: ");
    printHex(plain, cipherLen);
    Serial.println();

    printParsedValues(recordType, plain, cipherLen);
    Serial.println();
  }
};

static bool initDevices() {
  struct DeviceEntry {
    const char *name;
    const char *mac;
    const char *key;
  };

  const DeviceEntry entries[VICTRON_DEVICE_COUNT] = {
    {VICTRON_DEVICE_0_NAME, VICTRON_DEVICE_0_MAC, VICTRON_DEVICE_0_KEY},
    {VICTRON_DEVICE_1_NAME, VICTRON_DEVICE_1_MAC, VICTRON_DEVICE_1_KEY},
    {VICTRON_DEVICE_2_NAME, VICTRON_DEVICE_2_MAC, VICTRON_DEVICE_2_KEY},
    {VICTRON_DEVICE_3_NAME, VICTRON_DEVICE_3_MAC, VICTRON_DEVICE_3_KEY},
  };

  for (size_t i = 0; i < VICTRON_DEVICE_COUNT; i++) {
    devices[i].name = entries[i].name;
    devices[i].mac = entries[i].mac;
    devices[i].lastDataCounter = 0xFFFF;
    devices[i].configured = parseHexKey(entries[i].key, devices[i].key);

    if (!devices[i].configured) {
      Serial.print("ERROR: Invalid key for ");
      Serial.println(entries[i].name);
      return false;
    }

    Serial.print("Configured: ");
    Serial.print(entries[i].name);
    Serial.print(" (");
    Serial.print(entries[i].mac);
    Serial.println(")");
  }

  return true;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("SmartCamper test module - Victron multi-device BLE reader");
  Serial.println();

  if (!initDevices()) {
    return;
  }

  BLEDevice::init("");
  bleScan = BLEDevice::getScan();
  bleScan->setAdvertisedDeviceCallbacks(new VictronScanCallbacks());
  bleScan->setActiveScan(true);
  bleScan->setInterval(100);
  bleScan->setWindow(99);

  Serial.println();
  Serial.println("BLE scan started. Keep ESP32 close to the Victron devices.");
  Serial.println("Instant Readout via Bluetooth must be enabled on each device.");
  Serial.println();
}

void loop() {
  if (bleScan == nullptr) {
    delay(1000);
    return;
  }

  bleScan->start(BLE_SCAN_SECONDS, false);
  delay(200);
}
