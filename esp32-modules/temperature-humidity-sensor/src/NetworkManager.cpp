#include "NetworkManager.h"

NetworkManager::NetworkManager() {
    wifiConnected = false;
    mqttConnected = false;
}

void NetworkManager::setup() {
    mqttClient.setClient(espClient);
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setKeepAlive(MQTT_KEEPALIVE);
    
    if (DEBUG_SERIAL) {
        Serial.println("🔧 MQTT клиент конфигуриран");
        Serial.println("   Сървър: " + String(MQTT_SERVER));
        Serial.println("   Порт: " + String(MQTT_PORT));
        Serial.println("   Keep-alive: " + String(MQTT_KEEPALIVE) + "s");
    }
}

bool NetworkManager::connectWiFi() {
    if (DEBUG_SERIAL) {
        Serial.println("📡 Свързване с WiFi...");
    }
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(WIFI_RETRY_INTERVAL);
        if (DEBUG_SERIAL) {
            Serial.print(".");
        }
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        if (DEBUG_SERIAL) {
            Serial.println();
            Serial.println("✅ WiFi свързан успешно");
            Serial.println("IP адрес: " + WiFi.localIP().toString());
        }
        return true;
    } else {
        wifiConnected = false;
        if (DEBUG_SERIAL) {
            Serial.println();
            Serial.println("❌ WiFi свързване неуспешно!");
        }
        return false;
    }
}

bool NetworkManager::isWiFiConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void NetworkManager::reconnectWiFi() {
    if (DEBUG_SERIAL) {
        Serial.println("❌ WiFi връзката е изгубена, опитвам се да се свържа отново...");
    }
    connectWiFi();
}

bool NetworkManager::connectMQTT() {
    if (DEBUG_SERIAL) {
        Serial.println("📡 Свързване с MQTT broker...");
    }
    
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
        mqttConnected = true;
        if (DEBUG_SERIAL) {
            Serial.println("✅ MQTT свързан успешно");
        }
        return true;
    } else {
        mqttConnected = false;
        if (DEBUG_SERIAL) {
            Serial.println("❌ MQTT свързване неуспешно!");
        }
        return false;
    }
}

bool NetworkManager::isMQTTConnected() {
    return mqttClient.connected();
}

void NetworkManager::reconnectMQTT() {
    if (DEBUG_SERIAL) {
        Serial.println("❌ MQTT връзката е изгубена, опитвам се да се свържа отново...");
    }
    
    int attempts = 0;
    while (!mqttClient.connected() && attempts < 10) {
        if (DEBUG_SERIAL) {
            Serial.print("Опит ");
            Serial.print(attempts + 1);
            Serial.println("/10 за MQTT свързване...");
        }
        
        if (connectMQTT()) {
            if (DEBUG_SERIAL) {
                Serial.println("✅ MQTT пресвързване успешно!");
            }
            break;
        }
        
        attempts++;
        delay(3000); // По-кратка пауза между опитите
    }
    
    if (!mqttClient.connected()) {
        if (DEBUG_SERIAL) {
            Serial.println("❌ MQTT пресвързване неуспешно след 10 опита");
        }
    }
}

bool NetworkManager::publishMessage(const char* topic, const char* message) {
    if (!mqttClient.connected()) {
        return false;
    }
    
    return mqttClient.publish(topic, message);
}

void NetworkManager::loop() {
    mqttClient.loop();
}

void NetworkManager::maintainConnections() {
    // Проверка на WiFi връзката
    if (!isWiFiConnected()) {
        if (DEBUG_SERIAL) {
            Serial.println("🔄 Проверка на WiFi връзката...");
        }
        reconnectWiFi();
    }
    
    // Проверка на MQTT връзката - по-агресивна проверка
    if (!isMQTTConnected()) {
        if (DEBUG_SERIAL) {
            Serial.println("🔄 Проверка на MQTT връзката...");
        }
        reconnectMQTT();
    }
    
    // Допълнителна проверка - ако WiFi е OK, но MQTT не е свързан
    if (isWiFiConnected() && !isMQTTConnected()) {
        if (DEBUG_SERIAL) {
            Serial.println("⚠️ WiFi OK, но MQTT не е свързан - опитвам се да се свържа...");
        }
        connectMQTT();
    }
} 