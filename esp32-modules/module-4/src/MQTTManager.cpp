// MQTT Manager Implementation
// Universal MQTT manager for ESP32 modules

#include "MQTTManager.h"

MQTTManager::MQTTManager() {
  this->clientId = MQTT_CLIENT_ID_PREFIX + String(random(0xffff), HEX);
  this->brokerIP = MQTT_BROKER_IP;
  this->brokerPort = MQTT_BROKER_PORT;
  this->lastReconnectAttempt = 0;
  this->isConnected = false;
  this->failedAttempts = 0;
  this->lastWiFiWarningTime = 0;
  
  mqttClient.setClient(wifiClient);
}

MQTTManager::MQTTManager(String clientId, String brokerIP, int brokerPort) {
  this->clientId = clientId;
  this->brokerIP = brokerIP;
  this->brokerPort = brokerPort;
  this->lastReconnectAttempt = 0;
  this->isConnected = false;
  this->failedAttempts = 0;
  this->lastWiFiWarningTime = 0;
  
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
  // Check WiFi status directly
  bool wifiConnected = (WiFi.status() == WL_CONNECTED);
  loop(wifiConnected);
}

void MQTTManager::loop(bool wifiConnected) {
  if (!mqttClient.connected()) {
    // If WiFi is not connected, don't attempt MQTT reconnection
    if (!wifiConnected) {
      // Log only once every 5 seconds, not on every iteration
      unsigned long currentTime = millis();
      if (DEBUG_SERIAL && (currentTime - lastWiFiWarningTime > 5000)) {
        Serial.println("⚠️ MQTT: WiFi not connected, waiting...");
        lastWiFiWarningTime = currentTime;
      }
      failedAttempts = 0;  // Reset counter if WiFi is not connected
      return;
    }
    
    unsigned long currentTime = millis();
    if (currentTime - lastReconnectAttempt > MQTT_RECONNECT_DELAY) {
      lastReconnectAttempt = currentTime;
      bool connected = connect();
      
      if (connected) {
        failedAttempts = 0;  // Reset counter on successful connection
      } else {
        failedAttempts++;
        
        // If we have many failed attempts (10+), there might be a WiFi problem
        if (failedAttempts >= 10 && DEBUG_SERIAL) {
          Serial.println("⚠️ MQTT: Many failed attempts (" + String(failedAttempts) + "), check WiFi connection");
          Serial.println("WiFi Status: " + String(WiFi.status()));
          Serial.println("WiFi RSSI: " + String(WiFi.RSSI()) + " dBm");
        }
      }
    }
  } else {
    mqttClient.loop();
    failedAttempts = 0;  // Reset counter if connected
  }
}

bool MQTTManager::connect() {
  // Check if WiFi is connected before attempting MQTT reconnection
  if (WiFi.status() != WL_CONNECTED) {
    if (DEBUG_SERIAL) {
      Serial.println("⚠️ MQTT: Cannot connect - WiFi not connected");
    }
    isConnected = false;
    return false;
  }
  
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

bool MQTTManager::publishRaw(String topic, String payload) {
  if (!isMQTTConnected()) {
    if (DEBUG_SERIAL) {
      Serial.println("❌ Cannot publish - MQTT not connected");
    }
    return false;
  }
  
  bool result = mqttClient.publish(topic.c_str(), payload.c_str());
  
  if (DEBUG_MQTT) {
    if (result) {
      Serial.println("📤 Published: " + topic + " = " + payload);
    } else {
      Serial.println("❌ Failed to publish: " + topic + " = " + payload);
    }
  }
  
  return result;
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

int MQTTManager::getFailedAttempts() const {
  return failedAttempts;
}

void MQTTManager::printStatus() {
  if (DEBUG_SERIAL) {
    Serial.println("📊 MQTT Status:");
    Serial.println("  Connected: " + String(isMQTTConnected() ? "Yes" : "No"));
    Serial.println("  Client ID: " + clientId);
    Serial.println("  Broker: " + brokerIP + ":" + String(brokerPort));
    Serial.println("  State: " + String(mqttClient.state()));
    Serial.println("  Failed Attempts: " + String(failedAttempts));
  }
}

