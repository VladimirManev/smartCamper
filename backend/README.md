# SmartCamper Backend

Node.js server: **Express** (HTTP/static), **Socket.io** (WebSocket), **Aedes** (MQTT broker). Bridges MQTT messages from ESP32 modules to connected browsers and forwards UI commands to MQTT.

## Layout

| Path                      | Role                                                                   |
| ------------------------- | ---------------------------------------------------------------------- |
| `server.js`               | Express app, HTTP server, attaches Socket.io and Aedes                 |
| `socket/socketHandler.js` | WebSocket lifecycle, MQTT subscribe bridge, client command routing     |
| `socket/handlers/`        | Topic-specific logic (sensors, LEDs, commands, `moduleCommandHandler`) |
| `mqtt/`                   | Aedes broker setup                                                     |
| `src/ModuleRegistry.js`   | Tracks module online/offline from heartbeats                           |
| `src/history/`            | SQLite history logger (readings; `events` table reserved)              |
| `data/smartcamper.db`     | Runtime SQLite file (created on start; not committed)                  |

## WebSocket: client → server

The frontend emits these events; payloads are validated in each handler.

| Event                   | Purpose                                                    |
| ----------------------- | ---------------------------------------------------------- |
| `ledCommand`            | LED strips / relays (module-2)                             |
| `floorHeatingCommand`   | Floor heating (module-3)                                   |
| `levelingCommand`       | Leveling (module-3)                                        |
| `damperCommand`         | Dampers (module-4)                                         |
| `tableCommand`          | Table motor (module-4)                                     |
| `applianceCommand`      | Appliances (module-5)                                      |
| **`forceModuleUpdate`** | Request one ESP32 module to publish fresh data (see below) |

### `forceModuleUpdate`

Asks the backend to publish an MQTT **`force_update`** command to a **single** module.

- **Payload:** `{ "moduleId": "module-1" }` (string must match `^module-[1-9]\d*$`, e.g. `module-1`, `module-5`, `module-10`).
- **Invalid or missing `moduleId`:** silently ignored (no error event to client).
- **MQTT topic:** `smartcamper/commands/<moduleId>/force_update` with body `{}`.
- **Implementation:** `socket/socketHandler.js` → `sendForceUpdate()` in `socket/handlers/moduleCommandHandler.js`.

Used by the UI (e.g. gray water detail modal) to refresh module-1 sensor data on an interval without forcing every module.

On **new Socket.io connection**, the server still calls **`sendForceUpdateToAllOnline`** once (after a short delay) so the client gets data from every online module.

## WebSocket: server → client (common)

Emitted to all clients or to one client as noted in handlers:

| Event                      | Typical payload (summary)                       |
| -------------------------- | ----------------------------------------------- |
| `moduleStatusUpdate`       | `{ modules, timestamp }` — registry snapshot    |
| `sensorUpdate`             | Indoor/outdoor temp, humidity, gray water, clean water, toilet urine level, etc. |
| `ledStatusUpdate`          | LED / relay state (module-2)                    |
| `floorHeatingStatusUpdate` | Circles / full status (module-3)                |
| `levelingData`             | Pitch / roll (module-3)                         |
| `damperStatusUpdate`       | Damper angles (module-4)                        |
| `tableStatusUpdate`        | Table state (module-4)                          |
| `applianceStatusUpdate`    | Appliance relays (module-5)                     |
| `victronStatusUpdate`      | Victron energy snapshot (module-6)              |

## MQTT ↔ WebSocket

`aedes.on("publish", …)` forwards relevant topics through `heartbeatHandler` and `sensorDataHandler` (and related paths). Heartbeats update `ModuleRegistry` and may emit `moduleStatusUpdate`.

## History logging (SQLite)

Phase 1 test scope: write-only history for charts. Inspect with `sqlite3` / DB Browser until HTTP API exists.

| What | Rule |
| ---- | ---- |
| Victron `soc`, `voltage`, `current`, `solar_w` | On meaningful change (1 pp / 0.1 V / 0.5 A / 10 W) or at least every 60 s |
| Orion `orion_output_a` | Same as current (≥ 0.5 A change or 60 s) |
| `indoor_temp`, `indoor_humidity`, `outdoor_temp` | Every 5 minutes (last known values) |
| Retention | 30 days; purge on server start and every 24 h |

DB path defaults to `backend/data/smartcamper.db`. Override with `HISTORY_DB_PATH`. The `events` table is created but unused for now.

### HTTP API (read)

```http
GET /api/history/readings?metric=soc&hours=24
```

Returns `{ metric, unit, hours, points: [{ ts, value }] }` (points may be downsampled). Supported metrics include `soc`, `voltage`, `current`, `solar_w`, `orion_output_a`, climate metrics.

Example inspect:

```bash
sqlite3 backend/data/smartcamper.db "SELECT datetime(ts/1000,'unixepoch'), metric, value, unit FROM readings ORDER BY ts DESC LIMIT 20;"
```

## Environment

- `DEBUG_MQTT` — verbose MQTT logging when set.
- `HISTORY_DB_PATH` — optional path to the SQLite history file.

Frontend (production build served by this server) loads history via same origin. In Vite dev against the Pi, set `VITE_USE_PI_BACKEND=true` (same as Socket).

## Run

```bash
cd backend
npm install
npm start
# or: npm run dev
```

Default HTTP/WebSocket port is defined in `server.js` (typically **3000**). MQTT is usually on **1883**.

See also: Bulgarian summary in `README_BG.md`.
