// SmartCamper Backend Server
// Главен файл за Express сървъра

const express = require("express");

// Импортираме middleware-ите
const corsMiddleware = require("./middleware/cors");
const loggerMiddleware = require("./middleware/logger");

// Импортираме routes
const mainRoutes = require("./routes/main");
const notFoundRoutes = require("./routes/404");

// Създаваме Express приложение
const app = express();

// Middleware за парсиране на JSON данни
app.use(express.json());

// Наши custom middleware-и
app.use(corsMiddleware);
app.use(loggerMiddleware);

// Routes
app.use("/", mainRoutes);

// 404 handler - трябва да е последен!
app.use(notFoundRoutes);

// Стартираме сървъра на порт 3000
const PORT = 3000;
app.listen(PORT, () => {
  console.log(`🚀 SmartCamper Backend running on port ${PORT}`);
  console.log(`📡 Test: http://localhost:${PORT}`);
  console.log(`💚 Health: http://localhost:${PORT}/health`);
});
