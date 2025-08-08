#include "TiltSensor.h"
#include "config.h"

TiltSensor::TiltSensor(NetworkManager* networkManager, Adafruit_MPU6050* mpuSensor) 
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
    if (!mpu->begin()) {
        if (DEBUG_SERIAL) {
            Serial.println("❌ Неуспешно свързване с MPU6050!");
        }
        return;
    }
    
    // Конфигуриране на сензора
    mpu->setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu->setGyroRange(MPU6050_RANGE_500_DEG);
    mpu->setFilterBandwidth(MPU6050_BAND_21_HZ);
    
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
    
    // Четене на данните от сензора
    sensors_event_t a, g, temp;
    mpu->getEvent(&a, &g, &temp);
    
    // Изчисляване на ъглите от accelerometer данните
    float roll = atan2(a.acceleration.y, a.acceleration.z) * 180 / PI;  // Наклон напред/назад
    float pitch = atan2(-a.acceleration.x, sqrt(a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)) * 180 / PI; // Наклон наляво/надясно
    
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
    if (!mpu) return 0.0;
    
    sensors_event_t a, g, temp;
    mpu->getEvent(&a, &g, &temp);
    return atan2(a.acceleration.y, a.acceleration.z) * 180 / PI;
}

float TiltSensor::getPitch() {
    if (!mpu) return 0.0;
    
    sensors_event_t a, g, temp;
    mpu->getEvent(&a, &g, &temp);
    return atan2(-a.acceleration.x, sqrt(a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)) * 180 / PI;
}
