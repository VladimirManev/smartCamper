#include "SensorManager.h"

SensorManager::SensorManager(NetworkManager* network) {
    networkManager = network;
    lastCheckTime = 0;
    firstRead = true;
}

void SensorManager::loop() {
    if (shouldCheck()) {
        readAndPublish();
        updateCheckTime();
    }
}

bool SensorManager::shouldCheck() {
    return (millis() - lastCheckTime) >= 200; // Проверка на всеки 200ms
}

void SensorManager::updateCheckTime() {
    lastCheckTime = millis();
}

bool SensorManager::publishSensorData(const char* topic, float value, const char* unit, const char* deviceId) {
    // Данните вече са закръглени в DHTSensor, не ги закръгляме отново
    
    // Създаване на JSON
    StaticJsonDocument<200> doc;
    doc["value"] = value;
    doc["unit"] = unit;
    doc["device_id"] = deviceId;
    doc["timestamp"] = millis();
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    // Публикуване
    bool success = networkManager->publishMessage(topic, jsonString.c_str());
    
    if (DEBUG_SERIAL && !success) {
        Serial.println("❌ Грешка при публикуване на данни");
    }
    
    return success;
}

bool SensorManager::publishSensorData(const char* sensorType, float value, const char* unit, const char* sensorCategory, const char* deviceId) {
    // Създаване на topic в новия формат: smartcamper/sensors/{category}/{deviceId}/data
    String topic = "smartcamper/sensors/";
    topic += sensorCategory;
    topic += "/";
    topic += deviceId;
    topic += "/data";
    
    // Създаване на JSON
    StaticJsonDocument<200> doc;
    doc["value"] = value;
    doc["unit"] = unit;
    doc["sensor_type"] = sensorType;
    doc["device_id"] = deviceId;
    doc["timestamp"] = millis();
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    // Публикуване
    bool success = networkManager->publishMessage(topic.c_str(), jsonString.c_str());
    
    if (DEBUG_SERIAL && !success) {
        Serial.println("❌ Грешка при публикуване на данни");
    }
    
    return success;
} 