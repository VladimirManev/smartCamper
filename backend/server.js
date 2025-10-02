// SmartCamper Backend Server
// Главен файл за Express сървъра

const express = require("express");
const http = require("http");
const { Server } = require("socket.io");

// Импортираме middleware-ите
const corsMiddleware = require("./middleware/cors");
const loggerMiddleware = require("./middleware/logger");

// Импортираме routes
const mainRoutes = require("./routes/main");
const notFoundRoutes = require("./routes/404");

// Импортираме MQTT broker
const setupMQTTBroker = require("./mqtt/broker");

// Импортираме Socket.io handler
const setupSocketIO = require("./socket/socketHandler");

// Създаваме Express приложение
const app = express();

// Създаваме HTTP сървър (нужен за Socket.io)
const server = http.createServer(app);

// Създаваме Socket.io сървър
const io = new Server(server, {
  cors: {
    origin: "*", // Разрешава всички origins (за development)
    methods: ["GET", "POST"],
  },
});

// Middleware за парсиране на JSON данни
app.use(express.json());

// Наши custom middleware-и
app.use(corsMiddleware);
app.use(loggerMiddleware);

// Routes
app.use("/", mainRoutes);

// 404 handler - трябва да е последен!
app.use(notFoundRoutes);

// Инициализираме MQTT broker
const aedes = setupMQTTBroker();

// Инициализираме Socket.io с MQTT Bridge
setupSocketIO(io, aedes);

// Стартираме HTTP + WebSocket сървъра на порт 3000
const PORT = 3000;
server.listen(PORT, () => {
  console.log(`🚀 SmartCamper Backend running on port ${PORT}`);
  console.log(`📡 HTTP: http://localhost:${PORT}`);
  console.log(`💚 Health: http://localhost:${PORT}/health`);
  console.log(`🔌 WebSocket: ws://localhost:${PORT}`);
});
