const express = require("express");
const cors = require("cors");
const path = require("path");
const http = require("http");
const mqttBroker = require("./mqtt/broker");
const apiRoutes = require("./api/routes");
const app = express();
const server = http.createServer(app);
const PORT = process.env.PORT || 3000;

// CORS настройки
app.use(cors());
app.use(express.json());
app.use(express.urlencoded({ extended: true }));

// Статични файлове (React приложение)
app.use(express.static(path.join(__dirname, "public")));

// API маршрути
app.use("/api", apiRoutes);

// WebSocket setup за MQTT over WebSocket
mqttBroker.setupWebSocket(server);

// Catch-all route за React Router
app.get("*", (req, res) => {
  res.sendFile(path.join(__dirname, "public", "index.html"));
});

// Стартиране на сървъра
server.listen(PORT, () => {
  console.log(`🚀 SmartCamper сървър работи на порт ${PORT}`);
  console.log(`📡 MQTT Broker стартиран`);
  console.log(`🌐 Отворете: http://localhost:${PORT}`);
});

// Graceful shutdown
process.on("SIGTERM", () => {
  console.log("🛑 Сървърът се изключва...");
  process.exit(0);
});
