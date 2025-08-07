#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include "config.h"

// Обекти
DHT dht(DHT_PIN, DHT_TYPE);
WiFiClient espClient;
PubSubClient client(espClient);

// Променливи
unsigned long lastSensorRead = 0;
unsigned long lastMqttPublish = 0;
bool wifiConnected = false;
bool mqttConnected = false;

// Функции
void setupWiFi();
void setupMQTT();
void reconnectMQTT();
void readSensorData();
void publishSensorData(float temperature, float humidity);
void debugPrint(const char* message);

void setup() {
  // Инициализация на Serial
  Serial.begin(115200);
  delay(1000);
  
  debugPrint("🏕️ SmartCamper ESP32 - Temperature & Humidity Sensor");
  debugPrint("Starting up...");
  
  // Инициализация на DHT сензора
  dht.begin();
  debugPrint("DHT sensor initialized");
  
  // Настройване на WiFi
  setupWiFi();
  
  // Настройване на MQTT
  setupMQTT();
  
  debugPrint("Setup complete!");
}

void loop() {
  // Проверка на WiFi връзката
  if (WiFi.status() != WL_CONNECTED) {
    if (wifiConnected) {
      debugPrint("WiFi connection lost!");
      wifiConnected = false;
    }
    setupWiFi();
  } else if (!wifiConnected) {
    debugPrint("WiFi connected!");
    wifiConnected = true;
  }
  
  // Проверка на MQTT връзката
  if (!client.connected()) {
    if (mqttConnected) {
      debugPrint("MQTT connection lost!");
      mqttConnected = false;
    }
    reconnectMQTT();
  } else if (!mqttConnected) {
    debugPrint("MQTT connected!");
    mqttConnected = true;
  }
  
  // Поддържане на MQTT връзката
  client.loop();
  
  // Четене на сензорни данни
  unsigned long currentMillis = millis();
  if (currentMillis - lastSensorRead >= SENSOR_READ_INTERVAL) {
    readSensorData();
    lastSensorRead = currentMillis;
  }
  
  // Публикуване на данни
  if (currentMillis - lastMqttPublish >= MQTT_PUBLISH_INTERVAL) {
    // Данните се публикуват в readSensorData()
    lastMqttPublish = currentMillis;
  }
  
  delay(100); // Кратка пауза
}

void setupWiFi() {
  debugPrint("Connecting to WiFi...");
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    debugPrint("WiFi connected!");
    debugPrint("IP address: ");
    Serial.println(WiFi.localIP());
    wifiConnected = true;
  } else {
    debugPrint("WiFi connection failed!");
    wifiConnected = false;
  }
}

void setupMQTT() {
  client.setServer(MQTT_SERVER, MQTT_PORT);
  debugPrint("MQTT client configured");
}

void reconnectMQTT() {
  debugPrint("Attempting MQTT connection...");
  
  if (client.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
    debugPrint("MQTT connected!");
    mqttConnected = true;
  } else {
    debugPrint("MQTT connection failed!");
    mqttConnected = false;
  }
}

void readSensorData() {
  // Четене на температурата
  float temperature = dht.readTemperature();
  
  // Четене на влажността
  float humidity = dht.readHumidity();
  
  // Проверка дали четенето е успешно
  if (isnan(temperature) || isnan(humidity)) {
    debugPrint("Failed to read from DHT sensor!");
    return;
  }
  
  // Debug информация
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print("°C, Humidity: ");
  Serial.print(humidity);
  Serial.println("%");
  
  // Публикуване на данните
  publishSensorData(temperature, humidity);
}

void publishSensorData(float temperature, float humidity) {
  if (!client.connected()) {
    debugPrint("MQTT not connected, skipping publish");
    return;
  }
  
  // JSON за температурата
  StaticJsonDocument<200> tempDoc;
  tempDoc["value"] = temperature;
  tempDoc["unit"] = "celsius";
  tempDoc["device_id"] = "temp_living_01";
  tempDoc["timestamp"] = millis();
  
  String tempPayload;
  serializeJson(tempDoc, tempPayload);
  
  // JSON за влажността
  StaticJsonDocument<200> humDoc;
  humDoc["value"] = humidity;
  humDoc["unit"] = "percent";
  humDoc["device_id"] = "hum_living_01";
  humDoc["timestamp"] = millis();
  
  String humPayload;
  serializeJson(humDoc, humPayload);
  
  // Публикуване
  if (client.publish(MQTT_TOPIC_TEMPERATURE, tempPayload.c_str())) {
    debugPrint("Temperature data published");
  } else {
    debugPrint("Failed to publish temperature data");
  }
  
  if (client.publish(MQTT_TOPIC_HUMIDITY, humPayload.c_str())) {
    debugPrint("Humidity data published");
  } else {
    debugPrint("Failed to publish humidity data");
  }
}

void debugPrint(const char* message) {
  if (DEBUG_SERIAL) {
    Serial.print("[");
    Serial.print(millis());
    Serial.print("] ");
    Serial.println(message);
  }
} 