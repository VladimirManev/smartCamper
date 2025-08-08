#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <ArduinoJson.h>
#include "NetworkManager.h"

class SensorManager {
protected:
    NetworkManager* networkManager;
    unsigned long lastCheckTime;
    bool firstRead;
    
public:
    SensorManager(NetworkManager* network);
    virtual ~SensorManager() = default;
    
    // Основни функции
    virtual void setup() = 0;
    virtual void loop();
    virtual void readAndPublish() = 0;
    
    // Помощни функции
    bool shouldCheck();
    void updateCheckTime();
    bool publishSensorData(const char* topic, float value, const char* unit, const char* deviceId);
    bool publishSensorData(const char* sensorType, float value, const char* unit, const char* sensorCategory, const char* deviceId);
    
protected:
    // Виртуални функции за наследяване
    virtual float readSensorValue() = 0;
    virtual const char* getSensorUnit() = 0;
    virtual const char* getDeviceId() = 0;
    virtual const char* getTopic() = 0;
    virtual bool hasChanged(float newValue, float lastValue) = 0;
};

#endif 