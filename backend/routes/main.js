// Main Routes
// Main API endpoints

const express = require("express");
const router = express.Router();

// GET /health - health check endpoint
router.get("/health", (req, res) => {
  res.json({
    status: "healthy",
    uptime: process.uptime(),
    timestamp: new Date().toISOString(),
  });
});

module.exports = router;
