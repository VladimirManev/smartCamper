# SmartCamper - Управление на електрическата система

Интелигентна система за управление на електрическата система на кемпера с три основни компонента:

## 🏗️ Архитектура

### 1. **Backend (Мозък)**

- **Raspberry Pi 4** с Express.js сървър
- **MQTT Broker (Aedes)** за комуникация с модулите
- **Socket.io** за real-time WebSocket комуникация
- **MQTT ↔ WebSocket Bridge** за синхронизация между ESP32 модули и frontend

### 2. **Frontend (Дашборд)**

- **React** уеб приложение с Vite
- **Socket.io Client** за real-time обновления
- **Responsive** дизайн за мобилни устройства
- **Real-time** мониторинг на сензори и LED контроли

### 3. **ESP32 Модули**

- **PlatformIO** проектна структура
- **Arduino C++** код
- **MQTT** клиенти за комуникация
- **Модули**:
  - Temperature Sensor (DHT22/AM2301) - температура и влажност
  - LED Controller - управление на LED ленти с бутони, motion sensor и dimming

## 📁 Структура на проекта

```
smartCamper/
├── backend/              # Express.js сървър + Socket.io + MQTT
│   ├── server.js         # Главен сървър файл
│   ├── middleware/       # CORS, Logger, Static
│   ├── routes/           # API routes
│   ├── socket/           # Socket.io handler
│   └── mqtt/            # MQTT broker (Aedes)
├── frontend/             # React приложение (Vite)
│   ├── src/
│   │   ├── App.jsx      # Главен компонент
│   │   └── App.css      # Стилове
│   └── package.json
├── esp32-modules/        # ESP32 модули (PlatformIO)
│   ├── module-1/           # Module-1 (Температурен сензор, Water level)
│   ├── module-2/           # Module-2 (LED контролер)
│   └── module-3/           # Module-3 (Подово отопление контролер)
└── update-from-git.sh   # Скрипт за обновяване на Raspberry Pi
```

## 🚀 Стартиране

### Backend

```bash
cd backend
npm install
npm start
# или за development:
npm run dev
```

Backend стартира на порт **3000**:

- `http://localhost:3000` - главна страница
- `http://localhost:3000/health` - health check
- `ws://localhost:3000` - WebSocket сървър
- `mqtt://localhost:1883` - MQTT broker

### Frontend

```bash
cd frontend
npm install
npm run dev
```

Frontend стартира на порт **5174** (Vite dev server):

- `http://localhost:5174` - React dashboard

### ESP32 Модули

Използва се **PlatformIO** за компилация и качване:

```bash
cd esp32-modules/module-1
pio run --target upload

cd esp32-modules/module-2
pio run --target upload

cd esp32-modules/module-3
pio run --target upload
```

## 📡 Комуникация

- **MQTT**: ESP32 ↔ Backend (Aedes broker)
- **WebSocket**: Frontend ↔ Backend (Socket.io)
- **MQTT ↔ WebSocket Bridge**: Автоматична синхронизация на данни

### MQTT Topics

**Сензори:**

- `smartcamper/sensors/temperature` - температура
- `smartcamper/sensors/humidity` - влажност
- `smartcamper/sensors/module-2/status` - Module-2 (LED контролер) статус

**Команди:**

- `smartcamper/commands/module-2/strip/{index}/on` - включване на лента
- `smartcamper/commands/module-2/strip/{index}/off` - изключване на лента
- `smartcamper/commands/module-2/strip/{index}/brightness` - яркост

## 🔧 Технологии

- **Backend**: Node.js, Express.js, Socket.io, Aedes (MQTT)
- **Frontend**: React, Vite, Socket.io-client, Font Awesome
- **ESP32**: Arduino C++, PlatformIO, PubSubClient (MQTT), NeoPixelBus (LED)
- **DevOps**: PM2, systemd services

## 📚 Документация

- `backend/README.md` / `backend/README_BG.md` - Backend: WebSocket, MQTT мост, `forceModuleUpdate`
- `LED_CONTROLLER_DOCUMENTATION.md` - Пълна документация за LED контролера
- `RASPBERRY_PI_COMMANDS.md` - Команди за Raspberry Pi управление
- `update-from-git.sh` - Скрипт за обновяване на проекта на Raspberry Pi

## 🎯 Особености

- ✅ Real-time мониторинг на сензори
- ✅ LED контрол с бутони, dimming и transitions
- ✅ Motion sensor активация за баня
- ✅ Подово отопление с автоматичен температурен контрол (4 независими кръга)
- ✅ WiFi сигнал индикатор за всеки модул (подобен на мобилен телефон)
- ✅ Офлайн работа - backend сервира frontend
- ✅ Автоматично преподключване на WiFi и MQTT
- ✅ Модулна архитектура за лесно разширяване
