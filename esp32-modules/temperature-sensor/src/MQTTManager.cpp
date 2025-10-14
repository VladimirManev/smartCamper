// MQTT Manager Implementation
// Универсален MQTT мениджър за ESP32 модули

#include "MQTTManager.h"

MQTTManager::MQTTManager() {
  this->clientId = MQTT_CLIENT_ID_PREFIX + String(random(0xffff), HEX);
  this->brokerIP = MQTT_BROKER_IP;
  this->brokerPort = MQTT_BROKER_PORT;
  this->lastReconnectAttempt = 0;
  this->isConnected = false;
  
  mqttClient.setClient(wifiClient);
}

MQTTManager::MQTTManager(String clientId, String brokerIP, int brokerPort) {
  this->clientId = clientId;
  this->brokerIP = brokerIP;
  this->brokerPort = brokerPort;
  this->lastReconnectAttempt = 0;
  this->isConnected = false;
  
  mqttClient.setClient(wifiClient);
}

void MQTTManager::begin() {
  mqttClient.setServer(brokerIP.c_str(), brokerPort);
  
  if (DEBUG_SERIAL) {
    Serial.println("🔌 MQTT Manager initialized");
    Serial.println("Client ID: " + clientId);
    Serial.println("Broker: " + brokerIP + ":" + String(brokerPort));
  }
}

void MQTTManager::loop() {
  if (!mqttClient.connected()) {
    unsigned long currentTime = millis();
    if (currentTime - lastReconnectAttempt > MQTT_RECONNECT_DELAY) {
      lastReconnectAttempt = currentTime;
      connect();
    }
  } else {
    mqttClient.loop();
  }
}

bool MQTTManager::connect() {
  if (DEBUG_SERIAL) {
    Serial.println("🔄 Attempting MQTT connection...");
  }
  
  if (mqttClient.connect(clientId.c_str())) {
    isConnected = true;
    if (DEBUG_SERIAL) {
      Serial.println("✅ MQTT connected!");
    }
    return true;
  } else {
    isConnected = false;
    if (DEBUG_SERIAL) {
      Serial.println("❌ MQTT connection failed");
      Serial.println("State: " + String(mqttClient.state()));
    }
    return false;
  }
}

void MQTTManager::disconnect() {
  mqttClient.disconnect();
  isConnected = false;
  if (DEBUG_SERIAL) {
    Serial.println("📴 MQTT disconnected");
  }
}

bool MQTTManager::isMQTTConnected() {
  return mqttClient.connected();
}

bool MQTTManager::publishSensorData(String sensorType, String value) {
  if (!isMQTTConnected()) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ Cannot publish - MQTT not connected");
    }
    return false;
  }
  
  String topic = MQTT_TOPIC_SENSORS + sensorType;
  bool result = mqttClient.publish(topic.c_str(), value.c_str());
  
  if (DEBUG_MQTT) {
    if (result) {
      Serial.println("📤 Published: " + topic + " = " + value);
    } else {
      Serial.println("❌ Failed to publish: " + topic + " = " + value);
    }
  }
  
  return result;
}

bool MQTTManager::publishSensorData(String sensorType, float value) {
  return publishSensorData(sensorType, String(value));
}

bool MQTTManager::publishSensorData(String sensorType, int value) {
  return publishSensorData(sensorType, String(value));
}

bool MQTTManager::subscribeToCommands(String moduleType) {
  if (!isMQTTConnected()) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ Cannot subscribe - MQTT not connected");
    }
    return false;
  }
  
  String topic = MQTT_TOPIC_COMMANDS + moduleType + "/#";
  bool result = mqttClient.subscribe(topic.c_str());
  
  if (DEBUG_MQTT) {
    if (result) {
      Serial.println("📥 Subscribed to: " + topic);
    } else {
      Serial.println("❌ Failed to subscribe to: " + topic);
    }
  }
  
  return result;
}

void MQTTManager::setCallback(void (*callback)(char* topic, byte* payload, unsigned int length)) {
  mqttClient.setCallback(callback);
}

void MQTTManager::printStatus() {
  if (DEBUG_SERIAL) {
    Serial.println("📊 MQTT Status:");
    Serial.println("  Connected: " + String(isMQTTConnected() ? "Yes" : "No"));
    Serial.println("  Client ID: " + clientId);
    Serial.println("  Broker: " + brokerIP + ":" + String(brokerPort));
    Serial.println("  State: " + String(mqttClient.state()));
  }
}
