const express = require("express");
const { getSensorData } = require("../mqtt/broker");

const router = express.Router();

// GET /api/sensors - Всички сензорни данни
router.get("/sensors", (req, res) => {
  try {
    const allData = {
      temperature: getSensorData("temperature"),
      humidity: getSensorData("humidity"),
      waterLevel: getSensorData("waterLevel"),
      battery: getSensorData("battery"),
    };

    res.json({
      success: true,
      data: allData,
      timestamp: new Date().toISOString(),
    });
  } catch (error) {
    res.status(500).json({
      success: false,
      error: "Грешка при получаване на данни",
    });
  }
});

// GET /api/sensors/:type - Данни за конкретен тип сензор
router.get("/sensors/:type", (req, res) => {
  try {
    const { type } = req.params;
    const data = getSensorData(type);

    res.json({
      success: true,
      data: data,
      timestamp: new Date().toISOString(),
    });
  } catch (error) {
    res.status(500).json({
      success: false,
      error: "Грешка при получаване на данни",
    });
  }
});

// GET /api/sensors/:type/:deviceId - Данни за конкретно устройство
router.get("/sensors/:type/:deviceId", (req, res) => {
  try {
    const { type, deviceId } = req.params;
    const data = getSensorData(type, deviceId);

    if (!data) {
      return res.status(404).json({
        success: false,
        error: "Устройството не е намерено",
      });
    }

    res.json({
      success: true,
      data: data,
      timestamp: new Date().toISOString(),
    });
  } catch (error) {
    res.status(500).json({
      success: false,
      error: "Грешка при получаване на данни",
    });
  }
});

// GET /api/status - Статус на системата
router.get("/status", (req, res) => {
  res.json({
    success: true,
    status: "online",
    timestamp: new Date().toISOString(),
    version: "1.0.0",
  });
});

module.exports = router;
