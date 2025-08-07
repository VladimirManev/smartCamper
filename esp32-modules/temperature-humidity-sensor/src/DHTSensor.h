#ifndef DHT_SENSOR_H
#define DHT_SENSOR_H

#include <DHT.h>
#include "SensorManager.h"

class DHTSensor : public SensorManager {
private:
    DHT* dht;
    float lastTemperature;
    float lastHumidity;
    
public:
    DHTSensor(NetworkManager* network, DHT* dhtSensor);
    
    // Имплементация на виртуалните функции
    void setup() override;
    void readAndPublish() override;
    
protected:
    // Виртуални функции за температура
    float readSensorValue() override { return dht->readTemperature(); }
    const char* getSensorUnit() override { return "celsius"; }
    const char* getDeviceId() override { return "temp_living_01"; }
    const char* getTopic() override { return MQTT_TOPIC_TEMPERATURE; }
    bool hasChanged(float newValue, float lastValue) override;
    
    // Виртуални функции за влажност
    const char* getHumidityUnit() { return "percent"; }
    const char* getHumidityDeviceId() { return "hum_living_01"; }
    const char* getHumidityTopic() { return MQTT_TOPIC_HUMIDITY; }
    
private:
    void publishTemperature(float temperature);
    void publishHumidity(float humidity);
    float readTemperature();
    float readHumidity();
    bool hasTemperatureChanged(float newValue, float lastValue);
    bool hasHumidityChanged(float newValue, float lastValue);
};

#endif 