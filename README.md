# SmartCamper - Electrical System Management

Intelligent system for managing the electrical system of a camper with three main components:

## 🏗️ Architecture

### 1. **Backend (Brain)**

- **Raspberry Pi 4** with Express.js server
- **MQTT Broker (Aedes)** for module communication
- **Socket.io** for real-time WebSocket communication
- **MQTT ↔ WebSocket Bridge** for synchronization between ESP32 modules and frontend

### 2. **Frontend (Dashboard)**

- **React** web application with Vite
- **Socket.io Client** for real-time updates
- **Responsive** design for mobile devices
- **Real-time** monitoring of sensors and LED controls

### 3. **ESP32 Modules**

- **PlatformIO** project structure
- **Arduino C++** code
- **MQTT** clients for communication
- **Modules**:
  - Temperature Sensor (DHT22/AM2301) - temperature and humidity
  - LED Controller - LED strip control with buttons, motion sensor and dimming

## 📁 Project Structure

```
smartCamper/
├── backend/              # Express.js server + Socket.io + MQTT
│   ├── server.js         # Main server file
│   ├── middleware/       # CORS, Logger, Static
│   ├── routes/           # API routes
│   ├── socket/           # Socket.io handler
│   └── mqtt/            # MQTT broker (Aedes)
├── frontend/             # React application (Vite)
│   ├── src/
│   │   ├── App.jsx      # Main component
│   │   └── App.css      # Styles
│   └── package.json
├── esp32-modules/        # ESP32 modules (PlatformIO)
│   ├── temperature-sensor/  # Temperature sensor
│   └── led-controller/      # LED controller
└── update-from-git.sh   # Script for updating on Raspberry Pi
```

## 🚀 Getting Started

### Backend

```bash
cd backend
npm install
npm start
# or for development:
npm run dev
```

Backend runs on port **3000**:

- `http://localhost:3000` - main page
- `http://localhost:3000/health` - health check
- `ws://localhost:3000` - WebSocket server
- `mqtt://localhost:1883` - MQTT broker

### Frontend

```bash
cd frontend
npm install
npm run dev
```

Frontend runs on port **5174** (Vite dev server):

- `http://localhost:5174` - React dashboard

### ESP32 Modules

Use **PlatformIO** for compilation and upload:

```bash
cd esp32-modules/temperature-sensor
pio run --target upload

cd esp32-modules/led-controller
pio run --target upload
```

## 📡 Communication

- **MQTT**: ESP32 ↔ Backend (Aedes broker)
- **WebSocket**: Frontend ↔ Backend (Socket.io)
- **MQTT ↔ WebSocket Bridge**: Automatic data synchronization

### MQTT Topics

**Sensors:**

- `smartcamper/sensors/temperature` - temperature
- `smartcamper/sensors/humidity` - humidity
- `smartcamper/sensors/led-controller/status` - LED controller status

**Commands:**

- `smartcamper/commands/led-controller/strip/{index}/on` - turn on strip
- `smartcamper/commands/led-controller/strip/{index}/off` - turn off strip
- `smartcamper/commands/led-controller/strip/{index}/brightness` - brightness

## 🔧 Technologies

- **Backend**: Node.js, Express.js, Socket.io, Aedes (MQTT)
- **Frontend**: React, Vite, Socket.io-client, Font Awesome
- **ESP32**: Arduino C++, PlatformIO, PubSubClient (MQTT), NeoPixelBus (LED)
- **DevOps**: PM2, systemd services

## 📚 Documentation

- `backend/README.md` - Backend: WebSocket events, MQTT bridge, `forceModuleUpdate`
- `LED_CONTROLLER_DOCUMENTATION.md` - Complete LED controller documentation
- `RASPBERRY_PI_COMMANDS.md` - Raspberry Pi management commands
- `update-from-git.sh` - Script for updating project on Raspberry Pi

## 🎯 Features

- ✅ Real-time sensor monitoring
- ✅ LED control with buttons, dimming and transitions
- ✅ Motion sensor activation for bathroom
- ✅ Offline operation - backend serves frontend
- ✅ Automatic WiFi and MQTT reconnection
- ✅ Modular architecture for easy expansion
