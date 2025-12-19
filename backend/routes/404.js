// 404 Routes
// Handle unknown paths

const express = require("express");
const router = express.Router();

// 404 handler - must be last!
router.use((req, res) => {
  res.status(404).json({
    error: "Not Found",
    message: `Route ${req.originalUrl} not found`,
    timestamp: new Date().toISOString(),
  });
});

module.exports = router;
