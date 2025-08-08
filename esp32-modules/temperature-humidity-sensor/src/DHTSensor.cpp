#include "DHTSensor.h"

DHTSensor::DHTSensor(NetworkManager* network, DHT* dhtSensor) 
    : SensorManager(network) {
    dht = dhtSensor;
    lastTemperature = -999.0;
    lastHumidity = -999.0;
}

void DHTSensor::setup() {
    dht->begin();
    if (DEBUG_SERIAL) {
        Serial.println("✅ DHT сензор инициализиран");
    }
}

void DHTSensor::readAndPublish() {
    float temperature = readTemperature();
    float humidity = readHumidity();
    
    // Проверка дали данните са валидни
    if (isnan(temperature) || isnan(humidity)) {
        if (DEBUG_SERIAL) {
            Serial.println("❌ Грешка при четене на данни от DHT сензора!");
        }
        return;
    }
    
    // Закръгляне на данните
    temperature = round(temperature * 10.0) / 10.0; // До 1 знак след десетичната
    humidity = round(humidity); // До цяло число
    
    // Проверка за промяна в стойностите
    bool temperatureChanged = firstRead || hasTemperatureChanged(temperature, lastTemperature);
    bool humidityChanged = firstRead || hasHumidityChanged(humidity, lastHumidity);
    
    if (temperatureChanged || humidityChanged) {
        if (DEBUG_SERIAL) {
            Serial.println("📊 Публикуване на нови данни:");
        }
        
        // Публикуване на данните
        if (temperatureChanged) {
            if (DEBUG_SERIAL) {
                Serial.println("🌡️ Нова стойност на температура: " + String(temperature, 1) + "°C");
            }
            publishTemperature(temperature);
            lastTemperature = temperature;
        }
        
        if (humidityChanged) {
            if (DEBUG_SERIAL) {
                Serial.println("💧 Нова стойност на влажност: " + String(humidity, 0) + "%");
            }
            publishHumidity(humidity);
            lastHumidity = humidity;
        }
        
        firstRead = false;
    }
}

bool DHTSensor::hasChanged(float newValue, float lastValue) {
    return abs(newValue - lastValue) >= 0.05; // Намален праг за по-чувствително засичане
}

bool DHTSensor::hasTemperatureChanged(float newValue, float lastValue) {
    return abs(newValue - lastValue) >= 0.1; // Праг за температура
}

bool DHTSensor::hasHumidityChanged(float newValue, float lastValue) {
    return abs(newValue - lastValue) >= 1.0; // Праг за влажност - 1% (защото сега е закръглена до цяло число)
}

void DHTSensor::publishTemperature(float temperature) {
    publishSensorData("temperature", temperature, "celsius", "temperature", "living");
}

void DHTSensor::publishHumidity(float humidity) {
    publishSensorData("humidity", humidity, "percent", "humidity", "living");
}

float DHTSensor::readTemperature() {
    return dht->readTemperature();
}

float DHTSensor::readHumidity() {
    return dht->readHumidity();
} 