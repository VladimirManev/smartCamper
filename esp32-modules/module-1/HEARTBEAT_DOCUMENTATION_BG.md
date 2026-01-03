# Документация на Heartbeat Системата

## Преглед

Heartbeat системата предоставя стандартизиран начин за ESP32 модулите да докладват своя статус към backend. Това гарантира, че backend може да проследява кои модули са онлайн и да открива кога модул е офлайн.

## Архитектура

### Компоненти

1. **HeartbeatManager** - Самостоятелен клас, отговорен за heartbeat функционалността
2. **MQTT Комуникация** - Heartbeat съобщенията се изпращат чрез MQTT на отделен topic
3. **Backend Мониторинг** - Backend проследява последния heartbeat timestamp за всеки модул

### Принципи на Дизайна

- **Разделение на Отговорностите**: Heartbeat логиката е изолирана в собствен клас
- **Преизползваемост**: Същата имплементация във всички ESP32 модули
- **Независимост**: Heartbeat работи независимо от публикуването на данни от сензорите
- **Ниско Натоварване**: Минимално използване на CPU и мрежа

## Структура на MQTT Topic

### Heartbeat Topic

```
smartcamper/heartbeat/{module-id}
```

Примери:
- `smartcamper/heartbeat/module-1`
- `smartcamper/heartbeat/module-2`
- `smartcamper/heartbeat/module-3`

### Heartbeat Payload

JSON формат:
```json
{
  "timestamp": 1234567890,
  "moduleId": "module-1",
  "uptime": 3600,
  "wifiRSSI": -65
}
```

Полета:
- `timestamp`: Unix timestamp (милисекунди от стартиране, или реално време ако е налично)
- `moduleId`: Идентификатор на модула (например, "module-1")
- `uptime`: Време на работа на модула в секунди
- `wifiRSSI`: Сила на WiFi сигнала в dBm (опционално)

## Конфигурация на Тайминга

| Настройка | Стойност | Описание |
|----------|----------|----------|
| **Heartbeat Интервал** | 10000 ms | Време между heartbeat съобщенията (10 секунди) |
| **Backend Проверка Интервал** | 5000 ms | Колко често backend проверява за липсващи heartbeats |
| **Timeout Праг** | 25000 ms | Време след което модулът се счита за офлайн (2.5x heartbeat интервал) |

## HeartbeatManager API

### Инициализация

```cpp
HeartbeatManager heartbeatManager(&mqttManager, MODULE_ID);
heartbeatManager.begin();
```

### Главен Цикъл

```cpp
void loop() {
  // Обновяване на heartbeat (извикване в главния цикъл)
  heartbeatManager.loop();
}
```

### Методи

- `begin()` - Инициализира heartbeat мениджъра
- `loop()` - Обновява heartbeat (извикване в главния цикъл)
- `setModuleId(String id)` - Задава идентификатор на модула
- `forceSend()` - Принудително изпращане на heartbeat веднага
- `isEnabled()` - Проверява дали heartbeat е активиран
- `getLastSentTime()` - Взима timestamp на последното изпратено heartbeat

## Пример за Интеграция

```cpp
#include "HeartbeatManager.h"
#include "Config.h"

class SensorManager {
private:
  MQTTManager mqttManager;
  HeartbeatManager heartbeatManager;
  
public:
  SensorManager() : heartbeatManager(&mqttManager, MODULE_ID) {
    // ...
  }
  
  void begin() {
    mqttManager.begin();
    heartbeatManager.begin();
    // ...
  }
  
  void loop() {
    mqttManager.loop();
    heartbeatManager.loop();  // Обновяване на heartbeat
    // ... логика на сензорите
  }
};
```

## Поведение на Backend

1. **Получаване на Heartbeat**: Backend получава heartbeat на `smartcamper/heartbeat/{module-id}`
2. **Проследяване на Timestamp**: Backend съхранява последния heartbeat timestamp за всеки модул
3. **Периодична Проверка**: Backend проверява на всеки 5 секунди за липсващи heartbeats
4. **Откриване на Офлайн**: Ако не е получен heartbeat за 25 секунди, модулът се маркира като офлайн
5. **Излъчване на Статус**: Backend изпраща `moduleStatusUpdate` към frontend с всички статуси на модулите

## Обработка на Грешки

- **MQTT Не е Свързан**: Heartbeat се пропуска (няма грешка, ще опита отново когато е свързан)
- **WiFi Не е Свързан**: Heartbeat се пропуска (няма грешка, ще опита отново когато е свързан)
- **Неуспешно Публикуване**: Тиха грешка, ще опита отново на следващия интервал

## Анализ на Мрежов Трафик

За 10 модула с 10-секунден heartbeat интервал:
- Съобщения на минута: 60
- Байтове на съобщение: ~150 (включително MQTT overhead)
- Общо на минута: ~9 KB
- Общо на ден: ~13 MB

**Заключение**: Пренебрежимо мрежово въздействие за локална мрежа.

## Бъдещи Подобрения

- Last Will and Testament (LWT) за неочаквани разкачвания
- Heartbeat с допълнителна диагностична информация
- Адаптивни heartbeat интервали базирани на мрежови условия
- Heartbeat потвърждение от backend (опционално)

