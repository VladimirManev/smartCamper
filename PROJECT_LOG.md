# SmartCamper Development Log

## 📋 Проект: SmartCamper Electrical System Management

**Цел:** Система за управление на електрическата система на кемпер с три основни компонента:

- 🧠 **Brain (Raspberry Pi 4)** - главен сървър
- 📡 **ESP32 модули** - сензори (температура, влажност, наклон, и др.)
- 📱 **Dashboard** - уеб приложение за контрол и наблюдение

**Комуникация:** WiFi (MQTT за сензори, HTTP за dashboard)
**Режим:** Офлайн работа (backend сервира frontend)

---

## 🚀 Етап 1: Backend Setup (Express.js)

### Стъпка 1.1: Проектна структура

```
smartCamper/
├── backend/           # Express.js сървър
├── frontend/          # React приложение (бъдещо)
└── esp32-modules/     # ESP32 код (бъдещо)
```

### Стъпка 1.2: Backend инициализация

```bash
cd backend
npm init -y                    # Създава package.json
npm install express           # Инсталира Express.js
```

**Резултат:** `package.json` с Express dependency

### Стъпка 1.3: Основен сървър (server.js)

```javascript
const express = require("express");
const app = express();

// Middleware
app.use(express.json());

// Routes
app.get("/", (req, res) => {
  res.json({ message: "Hello from SmartCamper!" });
});

app.listen(3000, () => {
  console.log("Server running on port 3000");
});
```

**Тестване:** `npm start` → `http://localhost:3000`

### Стъпка 1.4: Модулна архитектура

**Проблем:** Всичко в един файл → неорганизирано
**Решение:** Разделяне на middleware и routes

**Структура:**

```
backend/
├── server.js              # Главен файл (конфигурация)
├── middleware/
│   ├── cors.js           # CORS middleware
│   └── logger.js         # Logging middleware
└── routes/
    ├── main.js           # Главни endpoints
    └── 404.js            # 404 обработка
```

### Стъпка 1.5: Middleware файлове

**middleware/cors.js:**

```javascript
const corsMiddleware = (req, res, next) => {
  res.header("Access-Control-Allow-Origin", "*");
  res.header("Access-Control-Allow-Headers", "Content-Type");
  next();
};
module.exports = corsMiddleware;
```

**middleware/logger.js:**

```javascript
const loggerMiddleware = (req, res, next) => {
  console.log(`[${new Date().toISOString()}] ${req.method} ${req.url}`);
  next();
};
module.exports = loggerMiddleware;
```

### Стъпка 1.6: Routes файлове

**routes/main.js:**

```javascript
const express = require("express");
const router = express.Router();

router.get("/", (req, res) => {
  res.json({
    message: "Hello from SmartCamper Backend!",
    status: "running",
    version: "1.0.0",
    timestamp: new Date().toISOString(),
  });
});

router.get("/health", (req, res) => {
  res.json({
    status: "healthy",
    uptime: process.uptime(),
    timestamp: new Date().toISOString(),
  });
});

module.exports = router;
```

**routes/404.js:**

```javascript
const express = require("express");
const router = express.Router();

router.use((req, res) => {
  res.status(404).json({
    error: "Not Found",
    message: `Route ${req.originalUrl} not found`,
    timestamp: new Date().toISOString(),
  });
});

module.exports = router;
```

### Стъпка 1.7: Финален server.js

```javascript
const express = require("express");
const corsMiddleware = require("./middleware/cors");
const loggerMiddleware = require("./middleware/logger");
const mainRoutes = require("./routes/main");
const notFoundRoutes = require("./routes/404");

const app = express();

app.use(express.json());
app.use(corsMiddleware);
app.use(loggerMiddleware);

app.use("/", mainRoutes);
app.use(notFoundRoutes);

const PORT = 3000;
app.listen(PORT, () => {
  console.log(`🚀 SmartCamper Backend running on port ${PORT}`);
  console.log(`📡 Test: http://localhost:${PORT}`);
  console.log(`💚 Health: http://localhost:${PORT}/health`);
});
```

---

## 📚 Научени концепции

### Express.js основи

- **App vs Router:** App е главното приложение, Router е под-приложение
- **Middleware:** Функции които се изпълняват преди routes
- **Routes:** URL endpoints и техните handlers

### Модулна архитектура

- **Разделяне на отговорности:** Всеки файл има една цел
- **Преизползваемост:** Можеш да използваш модули на различни места
- **Тестване:** Лесно тестване на отделни части

### Middleware ред

1. `express.json()` - парсира JSON заявки
2. `corsMiddleware` - обработва CORS
3. `loggerMiddleware` - логва заявки
4. Routes - обработва endpoints
5. 404 handler - улавя непознати пътища

---

## 🎯 Следващи стъпки

### Backend (бъдещи)

- [ ] Error handling middleware
- [ ] MQTT broker интеграция
- [ ] MongoDB за данни
- [ ] API endpoints за сензори
- [ ] WebSocket за real-time данни

### Frontend (бъдещи)

- [ ] React приложение
- [ ] Dashboard UI
- [ ] Real-time данни
- [ ] Мобилен responsive дизайн

### ESP32 модули (бъдещи)

- [ ] Температура и влажност сензор
- [ ] Наклон сензор (GY-521)
- [ ] MQTT клиент
- [ ] WiFi връзка

---

## 🔧 Полезни команди

```bash
# Backend
cd backend
npm start                    # Стартира сървъра
npm install <package>       # Инсталира пакет

# Тестване
curl http://localhost:3000           # Главна страница
curl http://localhost:3000/health    # Health check
curl http://localhost:3000/unknown   # 404 тест
```

---

## 🚀 Етап 2: Frontend Setup (React + Vite)

### Стъпка 2.1: Vite инициализация

```bash
cd frontend
npm create vite@latest . -- --template react
npm install
```

**Резултат:** React приложение с Vite build tool

### Стъпка 2.2: Основен React компонент

**frontend/src/App.jsx:**

```javascript
import { useState } from "react";

function App() {
  const [temperature, setTemperature] = useState(null);

  return (
    <div className="app">
      <h1>🚐 SmartCamper Dashboard</h1>
      <div className="sensor-card">
        <h2>🌡️ Температура</h2>
        <p>{temperature !== null ? `${temperature}°C` : "Зарежда..."}</p>
      </div>
    </div>
  );
}
```

**Обяснение:**

- `useState` = React hook за state management
- `temperature` = текуща стойност
- `setTemperature` = функция за промяна

---

## 🚀 Етап 3: WebSocket комуникация (Socket.io)

### Стъпка 3.1: Backend - Socket.io setup

**Инсталация:**

```bash
cd backend
npm install socket.io
```

**backend/server.js - модификация:**

```javascript
const http = require("http");
const { Server } = require("socket.io");

const server = http.createServer(app);
const io = new Server(server, {
  cors: { origin: "*", methods: ["GET", "POST"] }
});

// Вместо app.listen() използваме server.listen()
server.listen(PORT, () => { ... });
```

**backend/socket/socketHandler.js - нов файл:**

```javascript
const setupSocketIO = (io) => {
  io.on("connection", (socket) => {
    console.log("✅ Frontend се свърза");

    socket.emit("sensorUpdate", {
      temperature: 25.5,
      humidity: 60,
    });

    socket.on("disconnect", () => {
      console.log("❌ Frontend се изключи");
    });
  });
};

module.exports = setupSocketIO;
```

**Обяснение:**

- `http.createServer(app)` = създава HTTP сървър от Express app
- `new Server(server)` = добавя Socket.io към HTTP сървъра
- `io.on("connection")` = слуша за нови WebSocket връзки
- `socket.emit()` = изпраща данни към client

### Стъпка 3.2: Frontend - Socket.io client

**Инсталация:**

```bash
cd frontend
npm install socket.io-client
```

**frontend/src/App.jsx - с WebSocket:**

```javascript
import { useState, useEffect } from "react";
import io from "socket.io-client";

function App() {
  const [temperature, setTemperature] = useState(null);
  const [connected, setConnected] = useState(false);

  useEffect(() => {
    const socket = io("http://localhost:3000");

    socket.on("connect", () => {
      setConnected(true);
    });

    socket.on("sensorUpdate", (data) => {
      setTemperature(data.temperature);
    });

    return () => socket.disconnect();
  }, []);

  return (
    <div className="app">
      <h1>🚐 SmartCamper Dashboard</h1>
      <div className="status">
        <p>Статус: {connected ? "Онлайн ✅" : "Офлайн ❌"}</p>
      </div>
      <div className="sensor-card">
        <h2>🌡️ Температура</h2>
        <p>{temperature !== null ? `${temperature}°C` : "Зарежда..."}</p>
      </div>
    </div>
  );
}
```

**Обяснение:**

- `useEffect` = изпълнява се при mount на компонента
- `[]` dependency array = изпълни само веднъж
- `return () => {}` = cleanup функция при unmount
- `socket.on()` = слуша за events от backend

### Стъпка 3.3: Модулна структура за Socket.io

**Проблем:** Всичката Socket.io логика беше в server.js (70+ реда)

**Решение:** Изнесохме в отделен модул

**Финална структура:**

```
backend/
├── server.js              # Конфигурация (само 57 реда)
├── middleware/
│   ├── cors.js
│   └── logger.js
├── routes/
│   ├── main.js
│   └── 404.js
└── socket/
    └── socketHandler.js   # WebSocket логика
```

**backend/server.js - изчистен:**

```javascript
const setupSocketIO = require("./socket/socketHandler");

// ... middleware и routes ...

setupSocketIO(io);  // 1 ред вместо 30+

server.listen(PORT, ...);
```

**Предимства:**

- ✅ Separation of concerns
- ✅ По-лесно четене
- ✅ По-лесно тестване
- ✅ По-лесно разширяване

---

## 📚 Научени концепции (допълнение)

### React Hooks

**useState:**

```javascript
const [value, setValue] = useState(initialValue);
// value = текуща стойност
// setValue = функция за промяна
```

**useEffect:**

```javascript
useEffect(() => {
  // Код при mount
  return () => {
    // Cleanup при unmount
  };
}, []); // Dependency array
```

### Socket.io Pattern

**Backend:**

```javascript
io.on("connection", (socket) => {
  socket.emit("eventName", data); // Изпраща
  socket.on("eventName", handler); // Слуша
});
```

**Frontend:**

```javascript
const socket = io("url");
socket.on("eventName", (data) => { ... });
```

### Архитектурни принципи

**1. Backend като Gateway:**

- ESP32 → Backend (MQTT)
- Backend → Frontend (WebSocket)
- НИКОГА Frontend → ESP32 директно

**2. Протоколи:**

- WebSocket за real-time данни
- HTTP fetch за история/конфигурация
- MQTT за IoT устройства

**3. Модулност:**

- Всеки файл = една отговорност
- Лесно тестване
- Лесно поддръжка

---

## 🎯 Следващи стъпки (актуализирано)

### Backend

- [ ] MQTT broker (Mosquitto)
- [ ] MQTT ↔ WebSocket bridge
- [ ] MongoDB за история
- [ ] API endpoints за история
- [ ] Error handling middleware

### Frontend

- [ ] Още сензорни карти
- [ ] Графики (история)
- [ ] Контрол на релета
- [ ] Настройки
- [ ] Production build

### ESP32

- [ ] MQTT клиент
- [ ] Реални сензори
- [ ] WiFi connection
- [ ] Error handling

---

## 🔧 Полезни команди (актуализирано)

```bash
# Backend
cd backend
npm start                    # Стартира на :3000
npm install socket.io       # Socket.io

# Frontend
cd frontend
npm run dev                  # Стартира на :5173
npm install socket.io-client # Socket.io client
npm run build               # Production build

# Debugging
lsof -ti:3000 | xargs kill -9  # Убий процес на порт 3000
curl http://localhost:3000      # Тест HTTP
curl http://localhost:3000/health  # Health check
```

---

**Последно обновяване:** 2025-10-01
**Статус:** Backend + Frontend + WebSocket комуникация готови ✅
