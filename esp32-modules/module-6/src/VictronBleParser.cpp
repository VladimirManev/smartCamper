// Victron BLE Instant Readout parser implementation

#include "VictronBleParser.h"
#include "mbedtls/aes.h"
#include <cmath>
#include <cstring>

BitReader::BitReader(const uint8_t *data, size_t dataLen)
    : data(data), dataLen(dataLen), cursor(0) {}

bool BitReader::readBit() {
  if (cursor >= dataLen * 8) {
    return false;
  }
  size_t byteIndex = cursor / 8;
  uint8_t bitIndex = cursor % 8;
  bool value = (data[byteIndex] >> bitIndex) & 0x01;
  cursor++;
  return value;
}

uint32_t BitReader::readUnsigned(uint8_t bitCount) {
  uint32_t value = 0;
  for (uint8_t i = 0; i < bitCount; i++) {
    if (readBit()) {
      value |= (1U << i);
    }
  }
  return value;
}

int32_t BitReader::readSigned(uint8_t bitCount) {
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

void BitReader::skip(uint8_t bitCount) {
  for (uint8_t i = 0; i < bitCount; i++) {
    readBit();
  }
}

bool parseHexKey(const char *hexKey, uint8_t *outKey) {
  if (hexKey == nullptr || strlen(hexKey) != 32) {
    return false;
  }

  for (size_t i = 0; i < 16; i++) {
    char byteStr[3] = {hexKey[i * 2], hexKey[i * 2 + 1], '\0'};
    outKey[i] = (uint8_t)strtoul(byteStr, nullptr, 16);
  }
  return true;
}

bool isSupportedRecordType(uint8_t recordType) {
  return recordType == RECORD_BATTERY_MONITOR || recordType == RECORD_SOLAR_CHARGER ||
         recordType == RECORD_DCDC_CONVERTER || recordType == RECORD_ORION_XS ||
         recordType == RECORD_AC_CHARGER;
}

bool decryptVictronPayload(const uint8_t *key, const uint8_t *cipher, size_t cipherLen,
                           uint8_t counterLsb, uint8_t counterMsb, uint8_t *plainOut) {
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

  int result = mbedtls_aes_crypt_ctr(&aes, cipherLen, &ncOffset, nonce, streamBlock, cipher,
                                     plainOut);
  mbedtls_aes_free(&aes);
  return result == 0;
}

bool parseBatteryMonitor(const uint8_t *payload, size_t payloadLen, SmartShuntReading &out) {
  BitReader reader(payload, payloadLen);

  uint16_t timeToGo = (uint16_t)reader.readUnsigned(16);
  int16_t voltageRaw = (int16_t)reader.readSigned(16);
  uint16_t alarmReason = (uint16_t)reader.readUnsigned(16);

  reader.skip(16);

  reader.readUnsigned(2);
  uint32_t currentRawU = reader.readUnsigned(22);
  uint32_t consumedRaw = reader.readUnsigned(20);
  uint16_t socRaw = (uint16_t)reader.readUnsigned(10);

  out.timeToGoValid = timeToGo != 0xFFFF;
  out.timeToGoMin = timeToGo;

  out.voltageValid = voltageRaw != 0x7FFF;
  out.voltage = roundTo1Decimal(voltageRaw * 0.01f);

  out.alarmReason = alarmReason;

  out.currentValid = currentRawU != 0x3FFFFF;
  if (out.currentValid) {
    int32_t currentRaw = (int32_t)currentRawU;
    if (currentRawU & (1U << 21)) {
      currentRaw = (int32_t)(currentRawU | (~0U << 22));
    }
    out.current = roundTo2Decimals(currentRaw * 0.001f);
  }

  out.socValid = socRaw != 0x3FF;
  out.soc = (int)lroundf(socRaw * 0.1f);

  out.consumedAhValid = consumedRaw != 0xFFFFF;
  out.consumedAh = roundTo1Decimal(consumedRaw * -0.1f);

  return true;
}

bool parseSolarCharger(const uint8_t *payload, size_t payloadLen, MpptReading &out) {
  BitReader reader(payload, payloadLen);

  out.deviceState = (uint8_t)reader.readUnsigned(8);
  out.errorCode = (uint8_t)reader.readUnsigned(8);

  int16_t batteryVoltageRaw = (int16_t)reader.readSigned(16);
  int16_t batteryCurrentRaw = (int16_t)reader.readSigned(16);
  uint16_t yieldTodayRaw = (uint16_t)reader.readUnsigned(16);
  uint16_t pvPowerRaw = (uint16_t)reader.readUnsigned(16);

  reader.readUnsigned(9);

  out.batteryVoltageValid = batteryVoltageRaw != 0x7FFF;
  out.batteryVoltage = roundTo1Decimal(batteryVoltageRaw * 0.01f);

  out.batteryCurrentValid = batteryCurrentRaw != 0x7FFF;
  out.batteryCurrent = roundTo2Decimals(batteryCurrentRaw * 0.1f);

  out.yieldTodayValid = yieldTodayRaw != 0xFFFF;
  out.yieldTodayKwh = roundTo2Decimals(yieldTodayRaw * 0.01f);

  out.pvPowerValid = pvPowerRaw != 0xFFFF;
  out.pvPower = pvPowerRaw;

  return true;
}

bool parseOrionXs(const uint8_t *payload, size_t payloadLen, OrionReading &out) {
  BitReader reader(payload, payloadLen);

  out.deviceState = (uint8_t)reader.readUnsigned(8);
  out.errorCode = (uint8_t)reader.readUnsigned(8);

  int16_t outputVoltageRaw = (int16_t)reader.readSigned(16);
  int16_t outputCurrentRaw = (int16_t)reader.readSigned(16);
  uint16_t inputVoltageRaw = (uint16_t)reader.readUnsigned(16);
  uint16_t inputCurrentRaw = (uint16_t)reader.readUnsigned(16);
  out.offReason = reader.readUnsigned(32);

  out.outputVoltageValid = outputVoltageRaw != 0x7FFF;
  out.outputVoltage = roundTo1Decimal(outputVoltageRaw * 0.01f);

  out.outputCurrentValid = outputCurrentRaw != 0x7FFF;
  out.outputCurrent = roundTo2Decimals(outputCurrentRaw * 0.1f);

  out.inputVoltageValid = inputVoltageRaw != 0xFFFF;
  out.inputVoltage = roundTo1Decimal(inputVoltageRaw * 0.01f);

  out.inputCurrentValid = inputCurrentRaw != 0xFFFF;
  out.inputCurrent = roundTo2Decimals(inputCurrentRaw * 0.1f);

  return true;
}

bool parseAcCharger(const uint8_t *payload, size_t payloadLen, AcChargerReading &out) {
  BitReader reader(payload, payloadLen);

  out.deviceState = (uint8_t)reader.readUnsigned(8);
  out.errorCode = (uint8_t)reader.readUnsigned(8);

  uint32_t voltageRaw = reader.readUnsigned(13);
  uint32_t currentRaw = reader.readUnsigned(11);

  reader.skip(13 + 11 + 13 + 11 + 13 + 11 + 7);

  uint32_t acCurrentRaw = reader.readUnsigned(9);

  out.voltageValid = voltageRaw != 0x1FFF;
  out.voltage = roundTo1Decimal(voltageRaw * 0.01f);

  out.currentValid = currentRaw != 0x7FF;
  out.current = roundTo2Decimals(currentRaw * 0.1f);

  out.acCurrentValid = acCurrentRaw != 0x1FF;
  out.acCurrent = roundTo2Decimals(acCurrentRaw * 0.1f);

  return true;
}

float roundTo1Decimal(float value) {
  return roundf(value * 10.0f) / 10.0f;
}

float roundTo2Decimals(float value) {
  return roundf(value * 100.0f) / 100.0f;
}
