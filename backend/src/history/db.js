/**
 * SQLite database for SmartCamper history logging.
 * WAL mode for better concurrent read/write on SD card.
 */

const fs = require("fs");
const path = require("path");
const Database = require("better-sqlite3");

const DEFAULT_DB_PATH = path.join(__dirname, "../../data/smartcamper.db");

/** @type {import("better-sqlite3").Database|null} */
let sharedDb = null;
/** @type {string|null} */
let sharedDbPath = null;

/**
 * Open (or create) the history database and apply schema.
 * @param {string} [dbPath] - Absolute or relative path to .db file
 * @returns {import("better-sqlite3").Database}
 */
function openHistoryDb(dbPath = process.env.HISTORY_DB_PATH || DEFAULT_DB_PATH) {
  const resolved = path.resolve(dbPath);
  const dir = path.dirname(resolved);

  if (!fs.existsSync(dir)) {
    fs.mkdirSync(dir, { recursive: true });
  }

  const db = new Database(resolved);
  db.pragma("journal_mode = WAL");
  db.pragma("synchronous = NORMAL");

  db.exec(`
    CREATE TABLE IF NOT EXISTS readings (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      ts INTEGER NOT NULL,
      metric TEXT NOT NULL,
      value REAL NOT NULL,
      unit TEXT,
      source TEXT
    );

    CREATE INDEX IF NOT EXISTS idx_readings_metric_ts ON readings (metric, ts);
    CREATE INDEX IF NOT EXISTS idx_readings_ts ON readings (ts);

    CREATE TABLE IF NOT EXISTS events (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      ts INTEGER NOT NULL,
      type TEXT NOT NULL,
      module TEXT,
      origin TEXT,
      summary TEXT,
      payload_json TEXT
    );

    CREATE INDEX IF NOT EXISTS idx_events_ts ON events (ts);
    CREATE INDEX IF NOT EXISTS idx_events_type_ts ON events (type, ts);
  `);

  return db;
}

/**
 * Shared DB handle for logger + HTTP API (same process).
 * @param {string} [dbPath]
 * @returns {import("better-sqlite3").Database}
 */
function getHistoryDb(dbPath = process.env.HISTORY_DB_PATH || DEFAULT_DB_PATH) {
  const resolved = path.resolve(dbPath);
  if (sharedDb && sharedDbPath === resolved) {
    return sharedDb;
  }
  if (sharedDb) {
    try {
      sharedDb.close();
    } catch (_) {
      // ignore
    }
  }
  sharedDb = openHistoryDb(resolved);
  sharedDbPath = resolved;
  return sharedDb;
}

module.exports = {
  openHistoryDb,
  getHistoryDb,
  DEFAULT_DB_PATH,
};
