#include "AlarmManager.h"
#include <ArduinoJson.h>

AlarmManager::AlarmManager(ModuleManager* moduleMgr)
  : moduleManager(moduleMgr),
    alarmSystem(&buzzer),
    commandHandler(moduleMgr ? &moduleMgr->getMQTTManager() : nullptr, this, MODULE_ID) {
  if (moduleMgr == nullptr && DEBUG_SERIAL) {
    Serial.println("ERROR: AlarmManager: moduleManager cannot be nullptr!");
  }
}

void AlarmManager::begin() {
  if (DEBUG_SERIAL) {
    Serial.println("Alarm Manager starting...");
  }

  buzzer.begin();
  alarmSystem.begin();

  if (DEBUG_SERIAL) {
    Serial.println("Alarm Manager ready");
  }
}

void AlarmManager::loop() {
  buzzer.loop();
  alarmSystem.loop();

  if (alarmSystem.consumeStatusDirty()) {
    publishStatus();
  }
}

void AlarmManager::handleForceUpdate() {
  publishStatus();
}

void AlarmManager::publishStatus() {
  if (moduleManager == nullptr || !moduleManager->getMQTTManager().isMQTTConnected()) {
    return;
  }

  StaticJsonDocument<768> doc;

  JsonObject z1 = doc.createNestedObject("zone1");
  z1["armed"] = alarmSystem.isZone1Armed() || alarmSystem.isArmPending();
  z1["phase"] = alarmSystem.getPhaseString();
  z1["ignoreInteriorPir"] = alarmSystem.getIgnoreInteriorPir();
  z1["siren"] = alarmSystem.isSirenOn();
  z1["smoke"] = alarmSystem.isSmokeOn();

  JsonObject z2 = doc.createNestedObject("zone2");
  z2["armed"] = alarmSystem.isZone2Armed();

  JsonObject inputs = doc.createNestedObject("inputs");
  inputs["spareOpen"] = alarmSystem.isSpareOpen();
  inputs["interiorPir"] = alarmSystem.isInteriorPir();

  JsonObject doors = inputs.createNestedObject("doors");
  doors["driver"] = false;
  doors["passenger"] = false;
  doors["sliding"] = false;
  doors["rear"] = false;

  JsonObject perim = inputs.createNestedObject("perimeter");
  perim["front"] = alarmSystem.getPerimeterPir(0);
  perim["rear"] = alarmSystem.getPerimeterPir(1);
  perim["left_front"] = alarmSystem.getPerimeterPir(2);
  perim["left_rear"] = alarmSystem.getPerimeterPir(3);
  perim["right_front"] = alarmSystem.getPerimeterPir(4);
  perim["right_rear"] = alarmSystem.getPerimeterPir(5);

  doc["timestamp"] = millis() / 1000;

  String payload;
  serializeJson(doc, payload);

  String topic = String(MQTT_TOPIC_SENSORS) + MODULE_ID + "/status";
  moduleManager->getMQTTManager().publishRaw(topic, payload);

  if (DEBUG_MQTT) {
    Serial.println("Published: " + topic + " = " + payload);
  }
}
