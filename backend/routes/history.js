/**
 * History HTTP API (read-only for now).
 */

const express = require("express");
const { queryReadings } = require("../src/history/queryReadings");

const router = express.Router();

/**
 * GET /api/history/readings?metric=soc&hours=24
 */
router.get("/readings", (req, res) => {
  const metric =
    typeof req.query.metric === "string" ? req.query.metric.trim() : "";
  const hours = req.query.hours != null ? Number(req.query.hours) : 24;
  const maxPoints =
    req.query.maxPoints != null ? Number(req.query.maxPoints) : undefined;

  if (!metric) {
    return res.status(400).json({ error: "metric is required" });
  }

  try {
    const result = queryReadings({ metric, hours, maxPoints });
    return res.json(result);
  } catch (err) {
    if (err.code === "BAD_METRIC") {
      return res.status(400).json({ error: err.message });
    }
    console.log(`❌ History readings query failed: ${err.message}`);
    return res.status(500).json({ error: "Failed to query readings" });
  }
});

module.exports = router;
