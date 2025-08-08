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
    const float ROLL_THRESHOLD = 0.5;
    const float PITCH_THRESHOLD = 0.5;

public:
    TiltSensor(NetworkManager* networkManager, Adafruit_MPU6050* mpuSensor);
    
    void setup() override;
    void readAndPublish() override;
    
    bool hasRollChanged(float newRoll);
    bool hasPitchChanged(float newPitch);
    
    float getRoll();
    float getPitch();
};

#endif
