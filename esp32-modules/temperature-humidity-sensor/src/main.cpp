#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "config.h"
#include "NetworkManager.h"
#include "DHTSensor.h"
#include "TiltSensor.h"

// Обекти
DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_MPU6050 mpu;
NetworkManager networkManager;
DHTSensor dhtSensor(&networkManager, &dht);
TiltSensor tiltSensor(&networkManager, &mpu);

void setup() {
    if (DEBUG_SERIAL) {
        Serial.begin(115200);
        Serial.println("🚀 ESP32 SmartCamper Sensor стартира");
    }
    
    // Инициализация на мрежовия мениджър
    networkManager.setup();
    
    // Свързване с WiFi
    networkManager.connectWiFi();
    
    // Свързване с MQTT
    networkManager.connectMQTT();
    
    // Инициализация на DHT сензора
    dhtSensor.setup();
    
    // Инициализация на MPU6050 сензора
    tiltSensor.setup();
    
    if (DEBUG_SERIAL) {
        Serial.println("✅ Инициализация завършена");
    }
}

void loop() {
    // Поддържане на мрежовите връзки - проверяваме на всяка итерация
    networkManager.maintainConnections();
    
    // Обработка на MQTT съобщения
    networkManager.loop();
    
    // Четене и публикуване на сензорни данни
    dhtSensor.loop();
    tiltSensor.loop();
    
    // Кратка пауза за да не претоварваме процесора
    delay(100);
} 