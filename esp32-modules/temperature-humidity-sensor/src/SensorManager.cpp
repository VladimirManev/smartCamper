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
    return (millis() - lastCheckTime) >= 1000; // Проверка на всяка секунда
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
    
    if (DEBUG_SERIAL) {
        if (success) {
            // Показваме различни формати според единицата
            if (strcmp(unit, "percent") == 0) {
                Serial.println("✅ Данни публикувани: " + String(value, 0) + unit);
            } else {
                Serial.println("✅ Данни публикувани: " + String(value, 1) + unit);
            }
        } else {
            Serial.println("❌ Грешка при публикуване на данни");
        }
    }
    
    return success;
} 