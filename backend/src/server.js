const express = require("express");
const cors = require("cors");
const path = require("path");

// Импортираме нашите модули
const mqttBroker = require("./mqtt/broker");
const apiRoutes = require("./api/routes");

// Създаваме Express приложението
const app = express();
const PORT = process.env.PORT || 3000;

// Middleware за сигурност и CORS
app.use(cors());
app.use(express.json());
app.use(express.urlencoded({ extended: true }));

// Статични файлове (React приложението)
app.use(express.static(path.join(__dirname, "public")));

// API routes
app.use("/api", apiRoutes);

// Зареждаме React приложението за всички други routes
app.get("*", (req, res) => {
  res.sendFile(path.join(__dirname, "public", "index.html"));
});

// Стартираме сървъра
app.listen(PORT, () => {
  console.log(`🚀 SmartCamper сървър работи на порт ${PORT}`);
  console.log(`📡 MQTT Broker стартиран`);
  console.log(`🌐 Отворете: http://localhost:${PORT}`);
});

// Graceful shutdown
process.on("SIGTERM", () => {
  console.log("🛑 Сървърът се изключва...");
  process.exit(0);
});
