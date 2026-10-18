// Transparent Autoterm UART bridge + by-power inject + MQTT status.
// App may start/set_power only when off or already in by-power; STOP always.

#ifndef AUTOTERM_BRIDGE_H
#define AUTOTERM_BRIDGE_H

#include "MQTTManager.h"
#include "PacketDecoder.h"
#include "UartEndpoint.h"

class AutotermBridge {
 public:
  AutotermBridge();

  void begin(MQTTManager* mqtt);
  void loop();

  bool isOpen() const { return panel_.isOpen() && heater_.isOpen(); }

  void handleMQTTCommand(const String& message);
  void forceUpdate();

 private:
  void pollOne(UartEndpoint& source, UartEndpoint& peer, FrameAssembler& assembler, bool fromHeater);
  void onHeaterFrame(const uint8_t* frame, size_t len);
  void publishStatus(bool force);
  bool isControlLocked() const;
  const char* lockReason() const;
  bool injectTwice(const uint8_t* frame, size_t len);
  bool cmdStart(uint8_t power);
  bool cmdSetPower(uint8_t power);
  bool cmdStop();

  UartEndpoint panel_;
  UartEndpoint heater_;
  FrameAssembler panelAssembler_;
  FrameAssembler heaterAssembler_;
  MQTTManager* mqttManager_;

  const char* state_;
  const char* mode_;
  uint8_t power_;
  uint8_t errorCode_;
  int heaterTempC_;
  float batteryV_;
  bool hasStatus_;
  bool dirty_;
};

#endif
