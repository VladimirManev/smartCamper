#include "TiltSensor.h"
#include "config.h"

TiltSensor::TiltSensor(NetworkManager* networkManager, MPU6050* mpuSensor) 
    : SensorManager(networkManager), mpu(mpuSensor) {
    lastRoll = 0.0;
    lastPitch = 0.0;
    firstRead = true;
}

void TiltSensor::setup() {
    if (DEBUG_SERIAL) {
        Serial.println("🔧 Инициализация на MPU6050...");
    }
    
    // Инициализация на I2C
    Wire.begin(I2C_SDA, I2C_SCL);
    
    // Инициализация на MPU6050
    mpu->begin();
    
    // Калибрация на сензора
    if (DEBUG_SERIAL) {
        Serial.println("🔄 Калибриране на MPU6050...");
    }
    
    mpu->calcOffsets(true, true); // Калибриране на gyro и accelerometer
    
    if (DEBUG_SERIAL) {
        Serial.println("✅ MPU6050 инициализиран успешно");
    }
}

void TiltSensor::readAndPublish() {
    if (!mpu) {
        if (DEBUG_SERIAL) {
            Serial.println("❌ MPU6050 не е инициализиран!");
        }
        return;
    }
    
    // Обновяване на данните от сензора
    mpu->update();
    
    // Четене на ъглите
    float roll = mpu->getAngleX();  // Наклон напред/назад
    float pitch = mpu->getAngleY(); // Наклон наляво/надясно
    
    // Закръгляне до 1 десетична
    roll = round(roll * 10) / 10;
    pitch = round(pitch * 10) / 10;
    
    if (DEBUG_SERIAL) {
        Serial.print("📐 Roll: ");
        Serial.print(roll, 1);
        Serial.print("°, Pitch: ");
        Serial.print(pitch, 1);
        Serial.println("°");
    }
    
    // Проверка дали има промяна
    bool rollChanged = hasRollChanged(roll);
    bool pitchChanged = hasPitchChanged(pitch);
    
    if (firstRead || rollChanged || pitchChanged) {
        if (DEBUG_SERIAL) {
            Serial.println("📤 Публикуване на данни за наклона...");
        }
        
        // Публикуване на roll данни
        publishSensorData("roll", roll, "degrees", "tilt", "living");
        
        // Публикуване на pitch данни
        publishSensorData("pitch", pitch, "degrees", "tilt", "living");
        
        lastRoll = roll;
        lastPitch = pitch;
        firstRead = false;
        
        if (DEBUG_SERIAL) {
            Serial.println("✅ Данни за наклона публикувани");
        }
    }
}

bool TiltSensor::hasRollChanged(float newRoll) {
    return abs(newRoll - lastRoll) >= ROLL_THRESHOLD;
}

bool TiltSensor::hasPitchChanged(float newPitch) {
    return abs(newPitch - lastPitch) >= PITCH_THRESHOLD;
}

float TiltSensor::getRoll() {
    return mpu ? mpu->getAngleX() : 0.0;
}

float TiltSensor::getPitch() {
    return mpu ? mpu->getAngleY() : 0.0;
}
