// Victron BLE Instant Readout parser
// Decrypts and parses Victron manufacturer advertisement payloads

#ifndef VICTRON_BLE_PARSER_H
#define VICTRON_BLE_PARSER_H

#include <Arduino.h>
#include <cstddef>
#include <cstdint>

static const uint16_t VICTRON_VENDOR_ID = 0x02E1;
static const uint8_t VICTRON_BEACON_TYPE = 0x10;
static const uint8_t RECORD_BATTERY_MONITOR = 0x02;
static const uint8_t RECORD_SOLAR_CHARGER = 0x01;
static const uint8_t RECORD_DCDC_CONVERTER = 0x04;
static const uint8_t RECORD_ORION_XS = 0x0F;

struct SmartShuntReading {
  bool voltageValid;
  float voltage;
  bool currentValid;
  float current;
  bool socValid;
  int soc;
  bool consumedAhValid;
  float consumedAh;
  bool timeToGoValid;
  int timeToGoMin;
  uint16_t alarmReason;
};

struct MpptReading {
  uint8_t deviceState;
  uint8_t errorCode;
  bool batteryVoltageValid;
  float batteryVoltage;
  bool batteryCurrentValid;
  float batteryCurrent;
  bool pvPowerValid;
  int pvPower;
  bool yieldTodayValid;
  float yieldTodayKwh;
};

struct OrionReading {
  uint8_t deviceState;
  uint8_t errorCode;
  bool outputVoltageValid;
  float outputVoltage;
  bool outputCurrentValid;
  float outputCurrent;
  bool inputVoltageValid;
  float inputVoltage;
  bool inputCurrentValid;
  float inputCurrent;
  uint32_t offReason;
};

class BitReader {
 public:
  BitReader(const uint8_t *data, size_t dataLen);

  bool readBit();
  uint32_t readUnsigned(uint8_t bitCount);
  int32_t readSigned(uint8_t bitCount);
  void skip(uint8_t bitCount);

 private:
  const uint8_t *data;
  size_t dataLen;
  size_t cursor;
};

bool parseHexKey(const char *hexKey, uint8_t *outKey);
bool isSupportedRecordType(uint8_t recordType);
bool decryptVictronPayload(const uint8_t *key, const uint8_t *cipher, size_t cipherLen,
                           uint8_t counterLsb, uint8_t counterMsb, uint8_t *plainOut);
bool parseBatteryMonitor(const uint8_t *payload, size_t payloadLen, SmartShuntReading &out);
bool parseSolarCharger(const uint8_t *payload, size_t payloadLen, MpptReading &out);
bool parseOrionXs(const uint8_t *payload, size_t payloadLen, OrionReading &out);

float roundTo1Decimal(float value);
float roundTo2Decimals(float value);

#endif
