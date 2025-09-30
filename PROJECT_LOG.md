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

**Последно обновяване:** 2025-09-30
**Статус:** Backend основа завършена ✅
