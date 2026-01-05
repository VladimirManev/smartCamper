// Floor Heating Temperature Sensor
// Specific sensor logic for DS18B20 sensor (OneWire) for each heating circle
// Handles: Reading, averaging, change detection, publishing

#ifndef FLOOR_HEATING_SENSOR_H
#define FLOOR_HEATING_SENSOR_H

#include "Config.h"  // For CircleMode enum
#include "MQTTManager.h"
#include <OneWire.h>
#include <DallasTemperature.h>

// Forward declaration
class FloorHeatingController;

class FloorHeatingSensor {
private:
  MQTTManager* mqttManager;  // Reference to MQTT manager (not owned)
  uint8_t circleIndex;        // Which heating circle this sensor belongs to (0-3)
  uint8_t pin;                // GPIO pin for the sensor
  OneWire oneWire;
  DallasTemperature sensors;
  
  unsigned long lastSensorRead;
  unsigned long lastDataSent;
  float lastTemperature;
  bool forceUpdateRequested;
  bool lastMQTTState;  // Previous MQTT connection state (for detecting reconnects)
  
  // Error handling
  int failedReadCount;  // Counter for failed readings (3 failures = error)
  bool hasError;  // True if sensor has error (3 consecutive failures)
  
  // Temperature averaging
  float temperatureReadings[HEATING_TEMP_AVERAGE_COUNT];
  int temperatureIndex;
  int temperatureCount;
  unsigned long lastAverageTime;
  
  FloorHeatingController* controller;  // Reference to controller to check mode
  
  // Sensor reading functions
  float readTemperature();
  
  // Averaging functions
  float calculateAverageTemperature();
  
  // Publishing logic
  void publishIfNeeded(float temperature, unsigned long currentTime, bool forcePublish = false);

public:
  FloorHeatingSensor(MQTTManager* mqtt, uint8_t circleIndex, uint8_t pin);
  
  // Initialization
  void begin();
  
  // Main loop - call this in your main loop()
  void loop();
  
  // Force update
  void forceUpdate();
  
  // Set controller reference (to check if circle is in TEMP_CONTROL mode)
  void setController(FloorHeatingController* ctrl);
  
  // Status (const methods)
  float getLastTemperature() const { return lastTemperature; }
  unsigned long getLastDataSent() const { return lastDataSent; }
  bool isForceUpdateRequested() const { return forceUpdateRequested; }
  bool hasSensorError() const { return hasError; }
  uint8_t getCircleIndex() const { return circleIndex; }
  void printStatus() const;
};

#endif

