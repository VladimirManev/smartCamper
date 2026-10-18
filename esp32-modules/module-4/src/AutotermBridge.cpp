#include "AutotermBridge.h"

#include "AutotermCommands.h"
#include "Config.h"
#include "TrafficLogger.h"

#include <ArduinoJson.h>
#include <cmath>
#include <cstring>

namespace {

constexpr uint8_t kDefaultPower = 5;

bool pinsAreDistinct(int8_t a, int8_t b, int8_t c, int8_t d) {
  const int8_t pins[] = {a, b, c, d};
  for (int i = 0; i < 4; ++i) {
    for (int j = i + 1; j < 4; ++j) {
      if (pins[i] == pins[j]) {
        return false;
      }
    }
  }
  return true;
}

bool isThermostatMode(const char* mode) {
  return mode != nullptr &&
         (strcmp(mode, "by-heater-T") == 0 || strcmp(mode, "by-panel-T") == 0 ||
          strcmp(mode, "by-air-T") == 0);
}

bool isOffState(const char* state) {
  return state != nullptr && strcmp(state, "off") == 0;
}

bool isVentState(const char* state) {
  return state != nullptr && strcmp(state, "vent") == 0;
}

}  // namespace

AutotermBridge::AutotermBridge()
    : panel_(Serial1, "PANEL"),
      heater_(Serial2, "HEATER"),
      mqttManager_(nullptr),
      state_("unknown"),
      mode_("unknown"),
      power_(kDefaultPower),
      errorCode_(0),
      heaterTempC_(-127),
      batteryV_(-1.0f),
      hasStatus_(false),
      dirty_(false) {
  panelAssembler_.reset();
  heaterAssembler_.reset();
}

void AutotermBridge::begin(MQTTManager* mqtt) {
  mqttManager_ = mqtt;

  if (DEBUG_SERIAL) {
    Serial.println();
    Serial.println("AUTOTERM — bridge + by-power MQTT control");
    Serial.printf("UART %lu 8N1  PANEL RX%d TX%d  HEATER RX%d TX%d\n",
                  static_cast<unsigned long>(AUTOTERM_BAUD),
                  AUTOTERM_PANEL_RX_PIN,
                  AUTOTERM_PANEL_TX_PIN,
                  AUTOTERM_HEATER_RX_PIN,
                  AUTOTERM_HEATER_TX_PIN);
  }

  if (!pinsAreDistinct(AUTOTERM_PANEL_RX_PIN, AUTOTERM_PANEL_TX_PIN, AUTOTERM_HEATER_RX_PIN,
                       AUTOTERM_HEATER_TX_PIN)) {
    Serial.println("ERROR: Autoterm RX/TX pins overlap in Config.h");
    return;
  }

  panel_.start(AUTOTERM_PANEL_RX_PIN, AUTOTERM_PANEL_TX_PIN, AUTOTERM_BAUD, AUTOTERM_SERIAL_CONFIG,
               AUTOTERM_INVERT_UART, AUTOTERM_RX_PULLUP);
  heater_.start(AUTOTERM_HEATER_RX_PIN, AUTOTERM_HEATER_TX_PIN, AUTOTERM_BAUD, AUTOTERM_SERIAL_CONFIG,
                AUTOTERM_INVERT_UART, AUTOTERM_RX_PULLUP);

  if (DEBUG_SERIAL) {
    Serial.printf("Autoterm bridge %s\n", isOpen() ? "ready" : "FAILED to open UARTs");
  }
}

void AutotermBridge::loop() {
  pollOne(panel_, heater_, panelAssembler_, false);
  pollOne(heater_, panel_, heaterAssembler_, true);
  if (dirty_) {
    publishStatus(false);
  }
}

void AutotermBridge::pollOne(UartEndpoint& source, UartEndpoint& peer, FrameAssembler& assembler,
                             bool fromHeater) {
  if (!source.isOpen()) {
    return;
  }

  while (source.available() > 0) {
    const int raw = source.readByte();
    if (raw < 0) {
      break;
    }
    const uint8_t byte = static_cast<uint8_t>(raw);
    peer.writeByte(byte);

    uint8_t frame[5 + 255 + 2];
    size_t frameLen = 0;
    uint32_t firstUs = 0;
    const uint32_t timestamp = micros();
    if (!assembler.feed(byte, frame, sizeof(frame), frameLen, timestamp, firstUs)) {
      continue;
    }

#if AUTOTERM_LOG_DECODE
    char text[96];
    if (PacketDecoder::describe(frame, frameLen, text, sizeof(text))) {
      TrafficLogger::logDecoded(source.name(), firstUs, text);
    }
#endif

    if (fromHeater) {
      onHeaterFrame(frame, frameLen);
    }
  }
}

void AutotermBridge::onHeaterFrame(const uint8_t* frame, size_t len) {
  AutotermParsedStatus st;
  if (PacketDecoder::parseStatusReply(frame, len, st)) {
    if (state_ != st.state || errorCode_ != st.errorCode || heaterTempC_ != st.heaterTempC ||
        fabsf(batteryV_ - st.batteryV) > 0.05f || !hasStatus_) {
      dirty_ = true;
    }
    state_ = st.state;
    errorCode_ = st.errorCode;
    heaterTempC_ = st.heaterTempC;
    if (st.batteryV >= 0) {
      batteryV_ = st.batteryV;
    }
    hasStatus_ = true;
    return;
  }

  AutotermParsedSettings settings;
  if (PacketDecoder::parseSettingsReply(frame, len, settings)) {
    if (mode_ != settings.mode ||
        (settings.power >= 1 && settings.power <= 9 && power_ != settings.power) || !hasStatus_) {
      dirty_ = true;
    }
    mode_ = settings.mode;
    if (settings.power >= 1 && settings.power <= 9) {
      power_ = settings.power;
    }
    hasStatus_ = true;
  }
}

bool AutotermBridge::isControlLocked() const {
  if (isOffState(state_)) {
    return false;
  }
  if (isVentState(state_)) {
    return true;
  }
  return isThermostatMode(mode_);
}

const char* AutotermBridge::lockReason() const {
  if (isOffState(state_)) {
    return nullptr;
  }
  if (isVentState(state_)) {
    return "vent";
  }
  if (isThermostatMode(mode_)) {
    return "thermostat";
  }
  return nullptr;
}

void AutotermBridge::publishStatus(bool force) {
  if (!force && !dirty_) {
    return;
  }
  if (!mqttManager_ || !mqttManager_->isMQTTConnected()) {
    return;
  }

  StaticJsonDocument<320> doc;
  doc["state"] = state_;
  doc["mode"] = mode_;
  doc["power"] = power_;
  doc["errorCode"] = errorCode_;
  if (errorCode_ != 0) {
    const char* msg = PacketDecoder::errorMessage(errorCode_);
    if (msg != nullptr) {
      doc["error"] = msg;
    }
  }
  if (heaterTempC_ != -127) {
    doc["heaterTemp"] = heaterTempC_;
  }
  if (batteryV_ >= 0) {
    doc["battery"] = batteryV_;
  }
  // Locked only when Comfort Control put heater in vent/thermostat — app never starts those.
  const bool locked = isControlLocked();
  doc["controlLocked"] = locked;
  const char* reason = lockReason();
  if (reason != nullptr) {
    doc["lockReason"] = reason;
  }

  String payload;
  serializeJson(doc, payload);
  const String topic = MQTT_TOPIC_SENSORS + String("module-4/autoterm/status");
  if (mqttManager_->publishRaw(topic, payload)) {
    dirty_ = false;
    if (DEBUG_MQTT) {
      Serial.println("📤 Autoterm status: " + payload);
    }
  }
}

void AutotermBridge::forceUpdate() {
  publishStatus(true);
}

bool AutotermBridge::injectTwice(const uint8_t* frame, size_t len) {
  if (frame == nullptr || len == 0 || !heater_.isOpen()) {
    return false;
  }
  if (heater_.write(frame, len) != len) {
    return false;
  }
  delay(30);
  return heater_.write(frame, len) == len;
}

bool AutotermBridge::cmdStart(uint8_t power) {
  if (isControlLocked()) {
    if (DEBUG_SERIAL) {
      Serial.println("Autoterm: start refused (locked mode)");
    }
    return false;
  }
  if (power < 1 || power > 9) {
    power = power_ >= 1 && power_ <= 9 ? power_ : kDefaultPower;
  }
  uint8_t frame[16];
  const size_t len = AutotermCommands::buildStartByPower(power, frame, sizeof(frame));
  if (len == 0 || !injectTwice(frame, len)) {
    return false;
  }
  power_ = power;
  mode_ = "by-power";
  dirty_ = true;
  if (DEBUG_SERIAL) {
    Serial.printf("INJECT: START by-power %u\n", power);
  }
  return true;
}

bool AutotermBridge::cmdSetPower(uint8_t power) {
  if (isControlLocked() || isOffState(state_)) {
    if (DEBUG_SERIAL) {
      Serial.println("Autoterm: set_power refused");
    }
    return false;
  }
  if (strcmp(mode_, "by-power") != 0 && strcmp(mode_, "unknown") != 0) {
    return false;
  }
  if (power < 1 || power > 9) {
    return false;
  }
  uint8_t frame[16];
  const size_t len = AutotermCommands::buildSetPower(power, frame, sizeof(frame));
  if (len == 0 || !injectTwice(frame, len)) {
    return false;
  }
  power_ = power;
  mode_ = "by-power";
  dirty_ = true;
  if (DEBUG_SERIAL) {
    Serial.printf("INJECT: SET power %u\n", power);
  }
  return true;
}

bool AutotermBridge::cmdStop() {
  uint8_t frame[16];
  const size_t len = AutotermCommands::buildStop(frame, sizeof(frame));
  if (len == 0 || !injectTwice(frame, len)) {
    return false;
  }
  if (DEBUG_SERIAL) {
    Serial.println("INJECT: STOP");
  }
  return true;
}

void AutotermBridge::handleMQTTCommand(const String& message) {
  StaticJsonDocument<192> doc;
  const DeserializationError err = deserializeJson(doc, message);
  if (err) {
    if (DEBUG_SERIAL) {
      Serial.println("Autoterm: bad command JSON");
    }
    return;
  }
  if (doc["type"] != "autoterm") {
    return;
  }
  const char* action = doc["action"] | "";
  if (strcmp(action, "stop") == 0) {
    cmdStop();
    return;
  }
  if (strcmp(action, "start") == 0) {
    const uint8_t power = static_cast<uint8_t>(doc["power"] | power_);
    cmdStart(power);
    return;
  }
  if (strcmp(action, "set_power") == 0) {
    if (!doc.containsKey("power")) {
      return;
    }
    cmdSetPower(static_cast<uint8_t>(doc["power"] | 0));
  }
}
