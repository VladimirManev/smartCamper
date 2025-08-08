#ifndef TILT_SENSOR_H
#define TILT_SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "SensorManager.h"

class TiltSensor : public SensorManager {
private:
    Adafruit_MPU6050* mpu;
    float lastRoll;
    float lastPitch;
    bool firstRead;
    
    // Порог за промяна (градуси)
    const float ROLL_THRESHOLD = 1.0;
    const float PITCH_THRESHOLD = 1.0;

public:
    TiltSensor(NetworkManager* networkManager, Adafruit_MPU6050* mpuSensor);
    
    void setup() override;
    void readAndPublish() override;
    
    bool hasRollChanged(float newRoll);
    bool hasPitchChanged(float newPitch);
    
    float getRoll();
    float getPitch();
    
    // Имплементация на виртуалните функции от SensorManager
    float readSensorValue() override;
    const char* getSensorUnit() override;
    const char* getDeviceId() override;
    const char* getTopic() override;
    bool hasChanged(float newValue, float lastValue) override;
};

#endif
