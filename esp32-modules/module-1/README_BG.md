# Модул 1 - ESP32 Сензорен Модул

## Преглед

Модул 1 е ESP32-базиран сензорен модул, проектиран за системата SmartCamper. Предоставя мониторинг на температура и влажност чрез DHT22/AM2301 сензор, с разширяема архитектура, която позволява добавяне на допълнителни сензори.

## Архитектура

### Основни Компоненти

1. **ModuleManager** - Обща инфраструктура (Мрежа, MQTT, Heartbeat, Команди)
   - Управлява WiFi свързаност
   - Управлява MQTT комуникация
   - Изпраща heartbeat съобщения
   - Обработва команди от backend

2. **SensorManager** - Координатор на сензорите
   - Координира всички сензорни класове
   - Обработва заявки за принудително обновяване
   - Управлява жизнения цикъл на сензорите

3. **TemperatureHumiditySensor** - DHT22 сензорна имплементация
   - Чете температура и влажност
   - Открива промени и публикува данни
   - Обработва специфичната за сензора логика

### Принципи на Дизайна

- **Разделение на Отговорностите**: Инфраструктурата (ModuleManager) е отделена от логиката на сензорите
- **Разширяемост**: Лесно добавяне на нови сензори без промяна на съществуващия код
- **Преизползваемост**: ModuleManager може да се копира в други модули както е
- **Поддръжка**: Ясна структура и отговорности

## Хардуерна Конфигурация

### DHT22/AM2301 Сензор
- **Pin**: GPIO 25 (конфигурируемо в `Config.h`)
- **Тип**: DHT22 (конфигурируемо в `Config.h`)
- **Захранване**: 3.3V
- **Данни**: Цифров pin 25

## Софтуерна Конфигурация

### Идентификация на Модула
- **Module ID**: `module-1` (дефинирано в `Config.h`)

### MQTT Topics

**Heartbeat:**
```
smartcamper/heartbeat/module-1
```

**Данни от Сензори:**
```
smartcamper/sensors/temperature
smartcamper/sensors/humidity
```

**Команди:**
```
smartcamper/commands/module-1/force_update
```

### Конфигурация на Тайминга

- **Интервал за Четене на Сензор**: 1 секунда
- **Интервал на Heartbeat**: 10 секунди
- **Праг на Температура**: 0.1°C (откриване на промяна)
- **Праг на Влажност**: 1% (откриване на промяна)

## Добавяне на Нови Сензори

За да добавите нов сензор (например, ниво на вода):

1. Създайте нов сензорен клас:
```cpp
// include/WaterLevelSensor.h
class WaterLevelSensor {
  // Специфична за сензора логика
};
```

2. Добавете в SensorManager:
```cpp
// include/SensorManager.h
class SensorManager {
  TemperatureHumiditySensor temperatureHumiditySensor;
  WaterLevelSensor waterLevelSensor;  // Нов сензор
};
```

3. Инициализирайте и обновявайте в SensorManager:
```cpp
void SensorManager::loop() {
  temperatureHumiditySensor.loop();
  waterLevelSensor.loop();  // Нов сензор
}
```

## Компилиране и Качване

```bash
# Компилиране на проекта
pio run

# Качване на ESP32
pio run -t upload

# Мониториране на сериен изход
pio device monitor
```

## Зависимости

- `knolleary/PubSubClient@^2.8` - MQTT клиент
- `adafruit/DHT sensor library@^1.4.4` - Библиотека за DHT сензор
- `adafruit/Adafruit Unified Sensor@^1.1.14` - Унифицирана сензорна библиотека
- `bblanchon/ArduinoJson@^6.21.3` - JSON парсиране

## Структура на Файловете

```
module-1/
├── include/
│   ├── CommandHandler.h          # Обработка на команди
│   ├── HeartbeatManager.h        # Heartbeat функционалност
│   ├── ModuleManager.h           # Мениджър на инфраструктурата
│   ├── SensorManager.h           # Координатор на сензорите
│   └── TemperatureHumiditySensor.h # DHT сензорна логика
├── src/
│   ├── CommandHandler.cpp
│   ├── Config.h                  # Конфигурационни константи
│   ├── HeartbeatManager.cpp
│   ├── main.cpp                  # Точка на влизане
│   ├── ModuleManager.cpp
│   ├── MQTTManager.cpp/h         # MQTT комуникация
│   ├── NetworkManager.cpp/h       # WiFi управление
│   ├── SensorManager.cpp
│   └── TemperatureHumiditySensor.cpp
├── platformio.ini                # PlatformIO конфигурация
├── README.md                     # Този файл (на английски)
├── README_BG.md                  # Този файл (на български)
└── HEARTBEAT_DOCUMENTATION.md    # Документация за heartbeat системата
```

## Бележки

- Module ID се дефинира в `Config.h` като `MODULE_ID`
- DHT сензорният pin и тип са конфигурируеми в `Config.h`
- Всички специфични за сензора прагове са в `Config.h`
- ModuleManager е преизползваем във всички модули
- Сензорните класове са независими и могат лесно да се добавят/премахнат

## Подобрения и Безопасност

Модулът включва следните подобрения за production-ready код:

- **Null Pointer Validation**: Проверка за nullptr във всички критични места
- **Input Validation**: Валидация на входни параметри и стойности от сензорите
- **Error Handling**: Подобрена обработка на грешки с ясни съобщения
- **Initialization Checks**: Проверка на състоянието на инициализация
- **Const Correctness**: Използване на const методи където е подходящо
- **Bounds Checking**: Валидация на стойности от сензорите (температура: -40 до 80°C, влажност: 0-100%)

