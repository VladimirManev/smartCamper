// Temperature Sensor Manager Implementation
// Специфична логика за температурен сензор

#include "SensorManager.h"

SensorManager::SensorManager() {
  lastSensorRead = 0;
  lastTemperature = 0.0;
  lastHumidity = 0.0;
}

void SensorManager::begin() {
  Serial.begin(115200);
  Serial.println("🌡️ Temperature Sensor Module Starting...");
  
  // Инициализираме мрежата
  networkManager.begin();
  
  // Инициализираме MQTT
  mqttManager.begin();
  
  // Настройваме callback за команди
  mqttManager.setCallback([this](char* topic, byte* payload, unsigned int length) {
    this->handleMQTTMessage(topic, payload, length);
  });
  
  Serial.println("✅ Temperature Sensor Module Ready!");
}

void SensorManager::loop() {
  // Обновяваме мрежата
  networkManager.loop();
  
  // Обновяваме MQTT
  mqttManager.loop();
  
  // Четем сензорите на интервали
  unsigned long currentTime = millis();
  if (currentTime - lastSensorRead > SENSOR_READ_INTERVAL) {
    lastSensorRead = currentTime;
    
    if (networkManager.isWiFiConnected() && mqttManager.isMQTTConnected()) {
      // Генерираме симулирани данни
      float temperature = generateSimulatedTemperature();
      float humidity = generateSimulatedHumidity();
      
      // Публикуваме данните
      mqttManager.publishSensorData("temperature", temperature);
      mqttManager.publishSensorData("humidity", humidity);
      
      // Запазваме за сравнение
      lastTemperature = temperature;
      lastHumidity = humidity;
      
      Serial.println("📊 Sensor Data:");
      Serial.println("  Temperature: " + String(temperature) + "°C");
      Serial.println("  Humidity: " + String(humidity) + "%");
    }
  }
}

float SensorManager::generateSimulatedTemperature() {
  // Симулираме температура между 20-30°C с малки промени
  static float baseTemp = 25.0;
  static float direction = 0.1;
  
  baseTemp += direction;
  
  // Обръщаме посоката на границите
  if (baseTemp > 30.0) {
    baseTemp = 30.0;
    direction = -0.1;
  } else if (baseTemp < 20.0) {
    baseTemp = 20.0;
    direction = 0.1;
  }
  
  // Добавяме малко шум
  float noise = (random(-10, 11) / 100.0);
  return baseTemp + noise;
}

float SensorManager::generateSimulatedHumidity() {
  // Симулираме влажност между 40-80% с малки промени
  static float baseHumidity = 60.0;
  static float direction = 0.2;
  
  baseHumidity += direction;
  
  // Обръщаме посоката на границите
  if (baseHumidity > 80.0) {
    baseHumidity = 80.0;
    direction = -0.2;
  } else if (baseHumidity < 40.0) {
    baseHumidity = 40.0;
    direction = 0.2;
  }
  
  // Добавяме малко шум
  float noise = (random(-20, 21) / 100.0);
  return baseHumidity + noise;
}

void SensorManager::handleMQTTMessage(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.println("📨 Received MQTT command:");
  Serial.println("  Topic: " + String(topic));
  Serial.println("  Message: " + message);
  
  // Тук можеш да добавиш логика за команди
  // Например: включване/изключване на сензора
}

void SensorManager::printStatus() {
  Serial.println("📊 Temperature Sensor Status:");
  networkManager.printStatus();
  mqttManager.printStatus();
  Serial.println("  Last Temperature: " + String(lastTemperature) + "°C");
  Serial.println("  Last Humidity: " + String(lastHumidity) + "%");
}
