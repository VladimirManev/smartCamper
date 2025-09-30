// Main Routes
// Главна страница и основни endpoints

const express = require("express");
const router = express.Router();

// GET / - главна страница
router.get("/", (req, res) => {
  res.json({
    message: "Hello from SmartCamper Backend!",
    status: "running",
    version: "1.0.0",
    timestamp: new Date().toISOString(),
  });
});

// GET /health - health check endpoint
router.get("/health", (req, res) => {
  res.json({
    status: "healthy",
    uptime: process.uptime(),
    timestamp: new Date().toISOString(),
  });
});

module.exports = router;
