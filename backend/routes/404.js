// 404 Routes
// Обработка на непознати пътища

const express = require("express");
const router = express.Router();

// 404 handler - трябва да е последен!
router.use((req, res) => {
  res.status(404).json({
    error: "Not Found",
    message: `Route ${req.originalUrl} not found`,
    timestamp: new Date().toISOString(),
  });
});

module.exports = router;
