/**
 * Minimal Socket.io server that emits the same events as the real backend,
 * with static payloads so the frontend can be developed without MQTT / ESP32.
 *
 * Run from repo root:
 *   cd backend && node scripts/mockFrontendSocket.js
 *
 * Frontend: from localhost (e.g. http://localhost:5175) the dev app uses this
 * mock by default. Optional .env.local overrides:
 *   VITE_MOCK_BACKEND=false  +  VITE_USE_PI_BACKEND=true  →  http://192.168.4.1:3000
 *
 * Default URL: http://localhost:3100 (override with MOCK_SOCKET_PORT)
 * MOCK_CONNECT_DELAY_MS — wait before first emit burst (default 450).
 */

const http = require("http");
const { Server } = require("socket.io");

const PORT = Number(process.env.MOCK_SOCKET_PORT || 3100);

function ts() {
  return new Date().toISOString();
}

function buildModuleStatuses() {
  const modules = {};
  for (let i = 1; i <= 5; i++) {
    const moduleId = `module-${i}`;
    const now = Date.now();
    modules[moduleId] = {
      moduleId,
      status: "online",
      lastSeen: new Date(now).toISOString(),
      lastHeartbeat: now,
      uptime: 3600 + i * 120,
      wifiRSSI: -48 - i * 4,
      metadata: {
        moduleId,
        uptime: 3600,
        wifiRSSI: -48 - i * 4,
        timestamp: ts(),
      },
    };
  }
  return modules;
}

const STATIC = {
  sensorUpdate: {
    indoorTemperature: 22.4,
    indoorHumidity: 46,
    outdoorTemperature: 7.5,
    grayWaterLevel: 42,
    grayWaterTemperature: 17.2,
    timestamp: ts(),
  },
  ledStatusUpdate: {
    type: "full",
    data: {
      strips: {
        0: {
          state: "ON",
          brightness: 72,
          mode: "WHITE",
          channels: { r: 255, g: 255, b: 255, w: 255 },
          effect: "normal",
        },
        1: {
          state: "OFF",
          brightness: 0,
          mode: "OFF",
          channels: { r: 255, g: 255, b: 255, w: 255 },
          effect: "normal",
        },
        3: {
          state: "ON",
          brightness: 40,
          mode: "RGB",
          channels: { r: 200, g: 120, b: 40, w: 0 },
          effect: "normal",
        },
        4: {
          state: "ON",
          brightness: 55,
          mode: "RGB",
          channels: { r: 255, g: 230, b: 190, w: 0 },
          effect: "normal",
        },
      },
      relays: {
        0: { state: "OFF" },
      },
    },
    timestamp: ts(),
  },
  floorHeatingStatusUpdate: {
    type: "full",
    data: {
      circles: {
        0: { mode: "TEMP_CONTROL", relay: "ON", temperature: 23.1, error: false },
        1: { mode: "OFF", relay: "OFF", temperature: 20.5, error: false },
        2: { mode: "MANUAL", relay: "ON", temperature: 24.0, error: false },
        3: { mode: "OFF", relay: "OFF", temperature: 19.8, error: false },
      },
    },
    timestamp: ts(),
  },
  levelingData: {
    pitch: 1.35,
    roll: -0.82,
    timestamp: ts(),
  },
  tableStatusUpdate: {
    type: "table",
    direction: "stopped",
    autoMoving: false,
    timestamp: ts(),
  },
  applianceStatusUpdate: {
    type: "full",
    data: {
      relays: {
        0: { state: "ON" },
        1: { state: "OFF" },
        2: { state: "ON" },
        3: { state: "OFF" },
        4: { state: "OFF" },
        5: { state: "ON" },
      },
    },
    timestamp: ts(),
  },
};

function damperUpdates() {
  const angles = [45, 90, 45, 45, 0];
  return angles.map((angle, index) => ({
    type: "damper",
    index,
    angle,
    timestamp: ts(),
  }));
}

function randomSensorPayload() {
  const t = Date.now() / 1000;
  return {
    indoorTemperature: Math.round((21.5 + Math.sin(t / 17) * 1.8) * 10) / 10,
    indoorHumidity: Math.min(75, Math.max(30, Math.round(45 + Math.sin(t / 13) * 12))),
    outdoorTemperature: Math.round((7 + Math.cos(t / 19) * 4) * 10) / 10,
    grayWaterLevel: Math.min(95, Math.max(5, Math.round(40 + Math.sin(t / 23) * 15))),
    grayWaterTemperature: Math.round((17 + Math.sin(t / 21) * 2) * 10) / 10,
    timestamp: ts(),
  };
}

function sendAll(io, socket) {
  const stamp = ts();
  socket.emit("moduleStatusUpdate", {
    modules: buildModuleStatuses(),
    timestamp: stamp,
  });

  socket.emit("sensorUpdate", { ...STATIC.sensorUpdate, timestamp: stamp });
  socket.emit("ledStatusUpdate", {
    ...STATIC.ledStatusUpdate,
    data: STATIC.ledStatusUpdate.data,
    timestamp: stamp,
  });
  socket.emit("floorHeatingStatusUpdate", {
    ...STATIC.floorHeatingStatusUpdate,
    timestamp: stamp,
  });
  socket.emit("levelingData", { ...STATIC.levelingData, timestamp: stamp });
  for (const d of damperUpdates()) {
    socket.emit("damperStatusUpdate", { ...d, timestamp: stamp });
  }
  socket.emit("tableStatusUpdate", { ...STATIC.tableStatusUpdate, timestamp: stamp });
  socket.emit("applianceStatusUpdate", {
    ...STATIC.applianceStatusUpdate,
    timestamp: stamp,
  });

  // Client hooks may attach after the first burst; resend sensors shortly after connect.
  [150, 600].forEach((ms) => {
    setTimeout(() => {
      socket.emit("sensorUpdate", randomSensorPayload());
    }, ms);
  });
}

const server = http.createServer();
const io = new Server(server, {
  cors: { origin: true, credentials: true },
});

io.on("connection", (socket) => {
  console.log(`[mock] client connected ${socket.id}`);

  // Let the browser attach Socket.io listeners before the first burst (React useEffect).
  const connectDelayMs = Number(process.env.MOCK_CONNECT_DELAY_MS || 450);
  let sensorTimer = null;
  const connectTimer = setTimeout(() => {
    sendAll(io, socket);
    const sensorIntervalMs = Number(process.env.MOCK_SENSOR_INTERVAL_MS || 4000);
    sensorTimer = setInterval(() => {
      socket.emit("sensorUpdate", randomSensorPayload());
    }, sensorIntervalMs);
  }, connectDelayMs);

  const noop = [
    "ledCommand",
    "floorHeatingCommand",
    "levelingCommand",
    "damperCommand",
    "tableCommand",
    "applianceCommand",
    "forceModuleUpdate",
  ];
  for (const ev of noop) {
    socket.on(ev, (payload) => {
      if (process.env.DEBUG_MOCK_SOCKET) {
        console.log(`[mock] ${ev}`, payload);
      }
    });
  }

  socket.on("disconnect", (reason) => {
    clearTimeout(connectTimer);
    if (sensorTimer) clearInterval(sensorTimer);
    console.log(`[mock] client disconnected ${socket.id} (${reason})`);
  });
});

server.listen(PORT, () => {
  console.log(`[mock] Socket.io listening on http://localhost:${PORT}`);
  console.log(`[mock] Optional: MOCK_SENSOR_INTERVAL_MS (default 4000)`);
});
