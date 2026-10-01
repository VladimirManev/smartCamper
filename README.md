# SmartCamper - Electrical System Management

Intelligent system for managing the electrical system of a camper with three main components:

## Architecture

### 1. **Backend (Brain)**

- **Raspberry Pi 4** with Express.js server
- **MQTT Broker (Aedes)** for module communication
- **Socket.io** for real-time WebSocket communication
- **MQTT ↔ WebSocket Bridge** for synchronization between ESP32 modules and frontend
- **SQLite history** for energy/climate charts (`GET /api/history/readings`)

### 2. **Frontend (Dashboard)**

- **React** web application with Vite
- **Socket.io Client** for real-time updates
- Responsive UI (phone + tablet landscape)
- Energy diagram, appliances, lighting, climate, security, fridge BLE controls

### 3. **ESP32 Modules**

- **PlatformIO** + Arduino C++
- **MQTT** clients with heartbeat + `force_update`
- **Modules**:
  - **module-1** — indoor/outdoor temp, humidity, gray water level + temp
  - **module-2** — LED strips, motion, ambient relay
  - **module-3** — floor heating + leveling
  - **module-4** — dampers + table motor
  - **module-5** — appliance relays (fridge power, pump, inverter, …) + urine level + tablet backlight
  - **module-6** — Victron Instant Readout (SmartShunt, MPPTs, Orion, AC charger) + AAOBOSI fridge BLE
  - **module-7** — clean water level
  - **module-8** — security alarm (zones, siren, CAN doors)

## Project Structure

```
smartCamper/
├── backend/              # Express + Socket.io + Aedes MQTT + history
├── frontend/             # React (Vite) dashboard
├── esp32-modules/        # module-1 … module-8 (+ test scratch)
├── CAN_SIGNALS.md        # Ducato B-CAN notes
├── FUTURE_IDEAS.md       # Unscheduled ideas
└── update-from-git.sh    # Pi update helper
```

## Getting Started

### Backend

```bash
cd backend
npm install
npm start
# or: npm run dev
```

Backend on port **3000** (HTTP + WebSocket). MQTT usually **1883**.

### Frontend

```bash
cd frontend
npm install
npm run dev
```

Dev server: **5174**. Production build is served by the backend on the Pi.

### ESP32 Modules

```bash
cd esp32-modules/module-6   # example
pio run --target upload
```

See each module’s `README.md` / `README_BG.md` for wiring, MQTT topics, and config.

## Communication

- **MQTT**: ESP32 ↔ Backend (Aedes)
- **WebSocket**: Frontend ↔ Backend (Socket.io)
- Bridge: MQTT publishes → Socket.io events (`sensorUpdate`, `victronStatusUpdate`, `fridgeStatusUpdate`, …)

Typical topics use `smartcamper/sensors/<moduleId>/…` and `smartcamper/commands/<moduleId>/…`.

## Documentation

| Doc | Contents |
| --- | -------- |
| `backend/README.md` | WebSocket events, MQTT bridge, history API |
| `esp32-modules/module-*/README.md` | Per-module hardware + MQTT |
| `esp32-modules/module-6/FRIDGE_BLE.md` | Fridge BLE protocol + topics |
| `RASPBERRY_PI_COMMANDS.md` | Pi management |
| `FUTURE_IDEAS.md` | Ideas not yet scheduled |
| `DEVELOPMENT_PROMPT.md` / `QUICK_START_PROMPT.md` | Dev collaboration rules |

Bulgarian overview: `README_BG.md`.

## Features

- Real-time sensors, lighting, climate, appliances
- Victron energy snapshot (solar, shunt, Orion, **AC charger**)
- Fridge power (module-5 relay) + BLE mode/temps (module-6)
- Security alarm (module-8)
- Offline: backend serves the built frontend
- Auto WiFi / MQTT reconnect on ESP32 modules
