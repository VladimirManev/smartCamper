# SmartCamper Backend

Node.js server: **Express** (HTTP/static), **Socket.io** (WebSocket), **Aedes** (MQTT broker). Bridges MQTT messages from ESP32 modules to connected browsers and forwards UI commands to MQTT.

## Layout

| Path | Role |
|------|------|
| `server.js` | Express app, HTTP server, attaches Socket.io and Aedes |
| `socket/socketHandler.js` | WebSocket lifecycle, MQTT subscribe bridge, client command routing |
| `socket/handlers/` | Topic-specific logic (sensors, LEDs, commands, `moduleCommandHandler`) |
| `mqtt/` | Aedes broker setup |
| `src/ModuleRegistry.js` | Tracks module online/offline from heartbeats |

## WebSocket: client → server

The frontend emits these events; payloads are validated in each handler.

| Event | Purpose |
|-------|---------|
| `ledCommand` | LED strips / relays (module-2) |
| `floorHeatingCommand` | Floor heating (module-3) |
| `levelingCommand` | Leveling (module-3) |
| `damperCommand` | Dampers (module-4) |
| `tableCommand` | Table motor (module-4) |
| `applianceCommand` | Appliances (module-5) |
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

| Event | Typical payload (summary) |
|-------|---------------------------|
| `moduleStatusUpdate` | `{ modules, timestamp }` — registry snapshot |
| `sensorUpdate` | Indoor/outdoor temp, humidity, gray water, etc. |
| `ledStatusUpdate` | LED / relay state (module-2) |
| `floorHeatingStatusUpdate` | Circles / full status (module-3) |
| `levelingData` | Pitch / roll (module-3) |
| `damperStatusUpdate` | Damper angles (module-4) |
| `tableStatusUpdate` | Table state (module-4) |
| `applianceStatusUpdate` | Appliance relays (module-5) |

## MQTT ↔ WebSocket

`aedes.on("publish", …)` forwards relevant topics through `heartbeatHandler` and `sensorDataHandler` (and related paths). Heartbeats update `ModuleRegistry` and may emit `moduleStatusUpdate`.

## Environment

- `DEBUG_MQTT` — verbose MQTT logging when set.

## Run

```bash
cd backend
npm install
npm start
# or: npm run dev
```

Default HTTP/WebSocket port is defined in `server.js` (typically **3000**). MQTT is usually on **1883**.

See also: Bulgarian summary in `README_BG.md`.
