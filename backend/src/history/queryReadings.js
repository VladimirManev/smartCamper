/**
 * Query helpers for history readings (HTTP API).
 */

const { getHistoryDb } = require("./db");

const ALLOWED_METRICS = new Set([
  "soc",
  "voltage",
  "current",
  "solar_w",
  "orion_output_a",
  "indoor_temp",
  "indoor_humidity",
  "outdoor_temp",
]);

const DEFAULT_MAX_POINTS = 200;

/**
 * Downsample chronologically sorted points to at most maxPoints (keep first/last).
 * @param {Array<{ ts: number, value: number }>} points
 * @param {number} maxPoints
 */
function downsamplePoints(points, maxPoints) {
  if (points.length <= maxPoints) {
    return points;
  }

  const out = [];
  const lastIndex = points.length - 1;
  for (let i = 0; i < maxPoints; i++) {
    const index =
      i === maxPoints - 1
        ? lastIndex
        : Math.round((i * lastIndex) / (maxPoints - 1));
    const point = points[index];
    if (out.length === 0 || out[out.length - 1].ts !== point.ts) {
      out.push(point);
    }
  }
  return out;
}

/**
 * @param {Object} options
 * @param {string} options.metric
 * @param {number} [options.hours=24]
 * @param {number} [options.maxPoints=200]
 * @returns {{ metric: string, unit: string|null, hours: number, points: Array<{ ts: number, value: number }> }}
 */
function queryReadings({ metric, hours = 24, maxPoints = DEFAULT_MAX_POINTS }) {
  if (!ALLOWED_METRICS.has(metric)) {
    const err = new Error(`Unsupported metric: ${metric}`);
    err.code = "BAD_METRIC";
    throw err;
  }

  const safeHours = Math.min(Math.max(Number(hours) || 24, 1), 24 * 30);
  const safeMax = Math.min(Math.max(Number(maxPoints) || DEFAULT_MAX_POINTS, 10), 2000);
  const since = Date.now() - safeHours * 60 * 60 * 1000;
  const db = getHistoryDb();

  const rows = db
    .prepare(
      `SELECT ts, value, unit
       FROM readings
       WHERE metric = ? AND ts >= ?
       ORDER BY ts ASC`
    )
    .all(metric, since);

  const unit = rows.length > 0 ? rows[0].unit : null;
  const points = downsamplePoints(
    rows.map((r) => ({ ts: r.ts, value: r.value })),
    safeMax
  );

  return {
    metric,
    unit,
    hours: safeHours,
    points,
  };
}

module.exports = {
  ALLOWED_METRICS,
  queryReadings,
  downsamplePoints,
};
