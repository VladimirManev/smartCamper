// Temperature Sensor Manager Implementation
// Специфична логика за температурен сензор

#include "Config.h"
#include "SensorManager.h"

// Статичен указател към текущия инстанс
SensorManager* SensorManager::currentInstance = nullptr;

SensorManager::SensorManager() : dht(25, DHT22), commandHandler(&mqttManager, this, "temperature-sensor") {
  lastSensorRead = 0;
  lastDataSent = 0;              // Инициализираме последното изпращане
  lastTemperature = 0.0;
  lastHumidity = 0.0;
  forceUpdateRequested = false;
  
  // Задаваме текущия инстанс за статичните методи
  currentInstance = this;
}

void SensorManager::begin() {
  Serial.begin(115200);
  Serial.println("🌡️ Temperature Sensor Module Starting...");
  
  // Инициализираме DHT сензора
  dht.begin();
  Serial.println("🌡️ AM2301 DHT22 sensor initialized on pin 25");
  
  // Инициализираме мрежата
  networkManager.begin();
  
  // Инициализираме MQTT
  mqttManager.begin();
  
  // Настройваме callback за команди
  mqttManager.setCallback(handleMQTTMessage);
  
  // Инициализираме Command Handler
  commandHandler.begin();
  
  Serial.println("✅ Temperature Sensor Module Ready!");
}

void SensorManager::loop() {
  // Обновяваме мрежата
  networkManager.loop();
  
  // Обновяваме MQTT
  mqttManager.loop();
  
  // Обновяваме Command Handler
  commandHandler.loop();
  
  // Четем сензорите на интервали ИЛИ при force update
  unsigned long currentTime = millis();
  if (currentTime - lastSensorRead > SENSOR_READ_INTERVAL || forceUpdateRequested) {
    lastSensorRead = currentTime;
    
    if (networkManager.isWiFiConnected() && mqttManager.isMQTTConnected()) {
      // Четем реални данни от AM2301
      float temperature = readTemperature();
      float humidity = readHumidity();
      
      // Закръгляме данните
      temperature = round(temperature * 10) / 10;  // До 1 десетичен знак (23.4°C)
      humidity = round(humidity);                  // До цяло число (65%)
      
      // Публикуваме данните само ако са валидни
      if (!isnan(temperature) && !isnan(humidity)) {
        bool tempChanged = (abs(temperature - lastTemperature) >= TEMP_THRESHOLD);
        bool humidityChanged = (abs(humidity - lastHumidity) >= HUMIDITY_THRESHOLD);
        bool heartbeatNeeded = (currentTime - lastDataSent > HEARTBEAT_INTERVAL);
        
        // Публикуваме ако има промяна ИЛИ е нужен heartbeat ИЛИ е първото четене
        if (tempChanged || humidityChanged || heartbeatNeeded || lastTemperature == 0.0) {
          // Публикуваме само променените данни ИЛИ при heartbeat
          if (tempChanged || heartbeatNeeded || lastTemperature == 0.0) {
            mqttManager.publishSensorData("temperature", temperature);
            Serial.println("Published: smartcamper/sensors/temperature = " + String(temperature, 1));
          }
          
          if (humidityChanged || heartbeatNeeded || lastHumidity == 0.0) {
            mqttManager.publishSensorData("humidity", humidity);
            Serial.println("Published: smartcamper/sensors/humidity = " + String((int)humidity));
          }
          
          // Запазваме за сравнение
          lastTemperature = temperature;
          lastHumidity = humidity;
          lastDataSent = currentTime;  // Обновяваме времето на последното изпращане
        }
        // Ако няма промяна и не е нужен heartbeat - не печатаме нищо
        
        // Ресетираме force update флага
        forceUpdateRequested = false;
      } else {
        Serial.println("❌ Invalid sensor readings!");
        forceUpdateRequested = false;
      }
    }
  }
}

float SensorManager::readTemperature() {
  // Четем температура от AM2301
  float temp = dht.readTemperature();
  
  if (isnan(temp)) {
    Serial.println("❌ Failed to read temperature from AM2301");
    return NAN;
  }
  
  return temp;
}

float SensorManager::readHumidity() {
  // Четем влажност от AM2301
  float humidity = dht.readHumidity();
  
  if (isnan(humidity)) {
    Serial.println("❌ Failed to read humidity from AM2301");
    return NAN;
  }
  
  return humidity;
}

void SensorManager::handleForceUpdate() {
  forceUpdateRequested = true;
  if (DEBUG_SERIAL) {
    Serial.println("🚀 Force update requested - will read sensor on next loop");
  }
}

// Статичен MQTT callback метод
void SensorManager::handleMQTTMessage(char* topic, byte* payload, unsigned int length) {
  if (currentInstance) {
    currentInstance->commandHandler.handleMQTTMessage(topic, payload, length);
  }
}

void SensorManager::printStatus() {
  Serial.println("📊 Temperature Sensor Status:");
  Serial.println("  WiFi: " + String(networkManager.isWiFiConnected() ? "Connected" : "Disconnected"));
  Serial.println("  IP: " + networkManager.getLocalIP());
  mqttManager.printStatus();
  Serial.println("  Last Temperature: " + String(lastTemperature) + "°C");
  Serial.println("  Last Humidity: " + String(lastHumidity) + "%");
}
