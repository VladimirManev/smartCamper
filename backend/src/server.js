const express = require("express");
const cors = require("cors");
const path = require("path");
const mqttBroker = require("./mqtt/broker");
const apiRoutes = require("./api/routes");
const app = express();
const PORT = process.env.PORT || 3000;

// CORS настройки
app.use(cors());
app.use(express.json());
app.use(express.urlencoded({ extended: true }));

// Статични файлове (React приложение)
app.use(express.static(path.join(__dirname, "public")));

// API маршрути
app.use("/api", apiRoutes);

// Catch-all route за React Router
app.get("*", (req, res) => {
  res.sendFile(path.join(__dirname, "public", "index.html"));
});

// Стартиране на сървъра
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
