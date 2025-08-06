# SmartCamper Backend

Express.js сървър с вграден MQTT broker за управление на електрическата система на кемпера.

## 🚀 Стартиране

```bash
# Инсталиране на зависимости
npm install

# Стартиране в development режим
npm run dev

# Стартиране в production режим
npm start
```

## 📡 Порти

- **HTTP API**: 3000
- **MQTT Broker**: 1883

## 🔧 API Endpoints

### Сензорни данни

- `GET /api/sensors` - Всички сензорни данни
- `GET /api/sensors/:type` - Данни за конкретен тип сензор
- `GET /api/sensors/:type/:deviceId` - Данни за конкретно устройство

### Системен статус

- `GET /api/status` - Статус на системата

## 📡 MQTT Topics

### Получаване на данни

- `smartcamper/sensors/temperature/+/data`
- `smartcamper/sensors/humidity/+/data`
- `smartcamper/sensors/water-tank/+/level`
- `smartcamper/sensors/battery/+/voltage`

### Примерно MQTT съобщение

```json
{
  "topic": "smartcamper/sensors/temperature/living/data",
  "payload": {
    "value": 22.5,
    "unit": "celsius",
    "device_id": "temp_living_01"
  }
}
```

## 🏗️ Структура

```
backend/
├── src/
│   ├── server.js      # Основен Express сървър
│   ├── mqtt/
│   │   └── broker.js  # MQTT broker
│   ├── api/
│   │   └── routes.js  # API routes
│   └── public/        # React build файлове
└── package.json
```

## 🔌 Свързване с ESP32

ESP32 модулите трябва да се свържат към MQTT broker-а на порт 1883 и да публикуват данни в съответните topics.
