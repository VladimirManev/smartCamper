/**
 * History logger (Phase 1 test scope):
 * - Victron + Orion output current → readings (smart sample)
 * - Indoor temp/humidity + outdoor temp → readings every 5 minutes
 * - events table exists but is unused for now
 */

const { getHistoryDb } = require("./db");
const { selectVictronReadings } = require("./victronSampler");

const RETENTION_MS = 30 * 24 * 60 * 60 * 1000;
const PURGE_INTERVAL_MS = 24 * 60 * 60 * 1000;
const CLIMATE_INTERVAL_MS = 5 * 60 * 1000;

const CLIMATE_METRICS = {
  indoor_temp: { unit: "°C", source: "indoor" },
  indoor_humidity: { unit: "%", source: "indoor" },
  outdoor_temp: { unit: "°C", source: "outdoor" },
};

class HistoryLogger {
  constructor(options = {}) {
    this.db = getHistoryDb(options.dbPath);
    this.lastVictronByMetric = new Map();
    this.latestClimate = {
      indoor_temp: null,
      indoor_humidity: null,
      outdoor_temp: null,
    };
    this.climateTimer = null;
    this.purgeTimer = null;

    this.insertReadingStmt = this.db.prepare(
      `INSERT INTO readings (ts, metric, value, unit, source) VALUES (?, ?, ?, ?, ?)`
    );
    this.purgeStmt = this.db.prepare(`DELETE FROM readings WHERE ts < ?`);
    this.purgeEventsStmt = this.db.prepare(`DELETE FROM events WHERE ts < ?`);

    this.insertMany = this.db.transaction((rows) => {
      for (const row of rows) {
        this.insertReadingStmt.run(
          row.ts,
          row.metric,
          row.value,
          row.unit,
          row.source
        );
      }
    });
  }

  start() {
    this.purgeOld();
    this.purgeTimer = setInterval(() => this.purgeOld(), PURGE_INTERVAL_MS);
    if (this.purgeTimer.unref) {
      this.purgeTimer.unref();
    }

    this.climateTimer = setInterval(
      () => this.flushClimateSample(),
      CLIMATE_INTERVAL_MS
    );
    if (this.climateTimer.unref) {
      this.climateTimer.unref();
    }

    console.log("📜 History logger started (Victron + climate readings)");
  }

  stop() {
    if (this.climateTimer) {
      clearInterval(this.climateTimer);
      this.climateTimer = null;
    }
    if (this.purgeTimer) {
      clearInterval(this.purgeTimer);
      this.purgeTimer = null;
    }
    // Keep shared DB open for HTTP API until process exit
  }

  /**
   * Handle MQTT publish after the live WebSocket bridge path.
   * @param {string} topic
   * @param {string} message
   */
  handleMqtt(topic, message) {
    if (!topic || typeof message !== "string") {
      return;
    }

    if (topic === "smartcamper/sensors/module-6/status") {
      this.handleVictronStatus(message);
      return;
    }

    if (topic === "smartcamper/sensors/indoor-temperature") {
      this.cacheClimate("indoor_temp", parseFloat(message));
      return;
    }

    if (topic === "smartcamper/sensors/indoor-humidity") {
      this.cacheClimate("indoor_humidity", parseFloat(message));
      return;
    }

    if (topic === "smartcamper/sensors/outdoor-temperature") {
      this.cacheClimate("outdoor_temp", parseFloat(message));
    }
  }

  /**
   * @param {string} message - raw JSON string
   */
  handleVictronStatus(message) {
    let data;
    try {
      data = JSON.parse(message);
    } catch (_) {
      return;
    }
    if (!data || typeof data !== "object" || Array.isArray(data)) {
      return;
    }

    const now = Date.now();
    const rows = selectVictronReadings(data, this.lastVictronByMetric, now);
    if (rows.length === 0) {
      return;
    }

    const withTs = rows.map((r) => ({ ...r, ts: now }));
    try {
      this.insertMany(withTs);
      for (const r of withTs) {
        this.lastVictronByMetric.set(r.metric, { value: r.value, ts: r.ts });
      }
    } catch (err) {
      console.log(`❌ History Victron write failed: ${err.message}`);
    }
  }

  /**
   * @param {"indoor_temp"|"indoor_humidity"|"outdoor_temp"} metric
   * @param {number} value
   */
  cacheClimate(metric, value) {
    if (typeof value !== "number" || Number.isNaN(value)) {
      return;
    }
    this.latestClimate[metric] = value;
  }

  flushClimateSample() {
    const now = Date.now();
    const rows = [];

    for (const [metric, meta] of Object.entries(CLIMATE_METRICS)) {
      const value = this.latestClimate[metric];
      if (value == null) {
        continue;
      }
      rows.push({
        ts: now,
        metric,
        value,
        unit: meta.unit,
        source: meta.source,
      });
    }

    if (rows.length === 0) {
      return;
    }

    try {
      this.insertMany(rows);
    } catch (err) {
      console.log(`❌ History climate write failed: ${err.message}`);
    }
  }

  purgeOld() {
    const cutoff = Date.now() - RETENTION_MS;
    try {
      const readingsResult = this.purgeStmt.run(cutoff);
      const eventsResult = this.purgeEventsStmt.run(cutoff);
      const removed =
        (readingsResult.changes || 0) + (eventsResult.changes || 0);
      if (removed > 0) {
        console.log(`🧹 History purge: removed ${removed} row(s) older than 30 days`);
      }
    } catch (err) {
      console.log(`❌ History purge failed: ${err.message}`);
    }
  }
}

module.exports = HistoryLogger;
