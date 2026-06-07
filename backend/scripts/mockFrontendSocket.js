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
const AC_CHARGER_CYCLE_MS = 15000;
const VICTRON_STALE_MS = 6000;
let lastLoggedAcPhase = null;

function isMockAcChargerLive() {
  return Math.floor(Date.now() / AC_CHARGER_CYCLE_MS) % 2 === 0;
}

function isVictronDeviceStale(publishedAt, updatedAt) {
  if (publishedAt == null || updatedAt == null) return true;
  return publishedAt - updatedAt > VICTRON_STALE_MS;
}

function ts() {
  return new Date().toISOString();
}

function buildModuleStatuses() {
  const modules = {};
  for (let i = 1; i <= 7; i++) {
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
    cleanWaterLevel: 72,
    toiletUrineLevel: 50,
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
  victronStatusUpdate: {
    type: "full",
    data: {
      publishedAt: 45230,
      smartshunt: {
        voltage: 13.9,
        current: 7.31,
        soc: 99,
        consumedAh: -2.4,
        timeToGoMin: null,
        alarmReason: 0,
        updatedAt: 45100,
      },
      mppt1: {
        deviceState: 3,
        errorCode: 0,
        batteryVoltage: 13.9,
        batteryCurrent: 4.3,
        pvPower: 62,
        yieldTodayKwh: 0.22,
        updatedAt: 45080,
      },
      mppt2: {
        deviceState: 3,
        errorCode: 0,
        batteryVoltage: 13.9,
        batteryCurrent: 3.85,
        pvPower: 48,
        yieldTodayKwh: 0.18,
        updatedAt: 45110,
      },
      orion: {
        deviceState: 0,
        errorCode: 0,
        outputVoltage: 13.8,
        outputCurrent: 0,
        inputVoltage: 12.5,
        inputCurrent: 0,
        offReason: 129,
        updatedAt: 45120,
      },
      acCharger: null,
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
    cleanWaterLevel: Math.min(95, Math.max(10, Math.round(68 + Math.sin(t / 27) * 22))),
    toiletUrineLevel: [0, 50, 100][Math.floor(t / 12) % 3],
    timestamp: ts(),
  };
}

function randomVictronPayload() {
  const t = Date.now() / 1000;
  const round1 = (v) => Math.round(v * 10) / 10;
  const round2 = (v) => Math.round(v * 100) / 100;

  const soc = Math.min(98, Math.max(8, Math.round(55 + Math.sin(t / 31) * 40)));
  const scenario = Math.floor(t / 18) % 5;
  const sun = 0.55 + 0.45 * Math.max(0, Math.sin(t / 25));

  let solar1Power = Math.round(320 * sun + Math.sin(t / 5) * 20);
  let solar2Power = Math.round(290 * sun + Math.cos(t / 6) * 15);
  let alternatorCurrent = Math.round((18 + Math.sin(t / 6) * 12) * 10) / 10;
  let orionOutputCurrent = Math.round((12 + Math.sin(t / 7) * 8) * 10) / 10;
  let acCurrent = Math.round((2.2 + Math.sin(t / 11) * 1.4) * 10) / 10;
  let loadsPower = Math.round(120 + 80 * (1 - soc / 100) + Math.sin(t / 4) * 30);

  switch (scenario) {
    case 0:
      solar1Power = 0;
      solar2Power = 0;
      alternatorCurrent = 0;
      orionOutputCurrent = 0;
      acCurrent = 0;
      break;
    case 1:
      alternatorCurrent = 0;
      orionOutputCurrent = 0;
      acCurrent = 0;
      break;
    case 2:
      solar1Power = 0;
      solar2Power = 0;
      alternatorCurrent = 0;
      orionOutputCurrent = 0;
      acCurrent = 0;
      loadsPower = Math.round(260 + Math.sin(t / 4) * 35);
      break;
    case 3:
      loadsPower = 0;
      break;
    case 4:
    default:
      if (Math.sin(t / 9) < 0) solar2Power = 0;
      if (Math.cos(t / 11) < 0) {
        alternatorCurrent = 0;
        orionOutputCurrent = 0;
      }
      if (Math.sin(t / 13) < -0.2) acCurrent = 0;
      if (Math.cos(t / 7) < 0) loadsPower = 0;
      break;
  }

  if (alternatorCurrent <= 0) orionOutputCurrent = 0;

  const acChargerLive = isMockAcChargerLive();
  const mockAcChargerCurrent = acChargerLive
    ? round2(8 + Math.sin(t / 4) * 2)
    : round2(8.5);

  const batteryVoltage = round1(12.6 + Math.sin(t / 15) * 0.4);
  const mppt1BatteryCurrent =
    solar1Power > 0 && batteryVoltage > 0
      ? round2((solar1Power / batteryVoltage) * 0.95)
      : 0;
  const mppt2BatteryCurrent =
    solar2Power > 0 && batteryVoltage > 0
      ? round2((solar2Power / batteryVoltage) * 0.95)
      : 0;
  const loadsCurrent =
    loadsPower > 0 && batteryVoltage > 0 ? round2(loadsPower / batteryVoltage) : 0;
  const publishedAt = Date.now() % 10000000;
  const baseUpdatedAt = publishedAt - Math.floor(Math.random() * 600);
  const acChargerUpdatedAt = acChargerLive
    ? baseUpdatedAt + 60
    : publishedAt - 12000;

  const shuntCurrent = round2(
    mppt1BatteryCurrent +
      mppt2BatteryCurrent +
      orionOutputCurrent +
      (acChargerLive ? mockAcChargerCurrent : 0) -
      loadsCurrent
  );
  const orionOutputVoltage = round1(14.1 + Math.sin(t / 9) * 0.3);
  const alternatorVoltage = round1(13.8 + Math.sin(t / 8) * 0.4);

  return {
    publishedAt,
    smartshunt: {
      voltage: batteryVoltage,
      current: shuntCurrent,
      soc,
      consumedAh: round1(-2.4 + Math.sin(Date.now() / 50000)),
      timeToGoMin: null,
      alarmReason: 0,
      updatedAt: baseUpdatedAt,
    },
    mppt1: {
      deviceState: solar1Power > 0 ? 3 : 0,
      errorCode: 0,
      batteryVoltage,
      batteryCurrent: mppt1BatteryCurrent,
      pvPower: Math.max(0, solar1Power),
      yieldTodayKwh: round2(0.15 + solar1Power / 2000),
      updatedAt: baseUpdatedAt + 20,
    },
    mppt2: {
      deviceState: solar2Power > 0 ? 3 : 0,
      errorCode: 0,
      batteryVoltage,
      batteryCurrent: mppt2BatteryCurrent,
      pvPower: Math.max(0, solar2Power),
      yieldTodayKwh: round2(0.12 + solar2Power / 2000),
      updatedAt: baseUpdatedAt + 35,
    },
    orion: {
      deviceState: orionOutputCurrent > 0 || alternatorCurrent > 0 ? 3 : 0,
      errorCode: 0,
      outputVoltage: orionOutputVoltage,
      outputCurrent: orionOutputCurrent,
      inputVoltage: alternatorVoltage,
      inputCurrent: alternatorCurrent,
      offReason: orionOutputCurrent > 0 ? 0 : 129,
      updatedAt: baseUpdatedAt + 50,
    },
    acCharger: {
      deviceState: 3,
      errorCode: 0,
      current: mockAcChargerCurrent,
      voltage: batteryVoltage,
      updatedAt: acChargerUpdatedAt,
    },
  };
}

function logAcChargerMockPhase(data) {
  const publishedAt = data?.publishedAt;
  const updatedAt = data?.acCharger?.updatedAt;
  const phase = isVictronDeviceStale(publishedAt, updatedAt) ? "OFF (stale)" : "LIVE";
  if (phase !== lastLoggedAcPhase) {
    lastLoggedAcPhase = phase;
    console.log(`[mock] AC charger signal: ${phase}`);
  }
}

function emitVictronStatus(socket, data) {
  logAcChargerMockPhase(data);
  socket.emit("victronStatusUpdate", {
    type: "full",
    data,
    timestamp: ts(),
  });
}

function createInitialLedState() {
  return JSON.parse(JSON.stringify(STATIC.ledStatusUpdate.data));
}

function emitLedStatus(socket, ledState) {
  socket.emit("ledStatusUpdate", {
    type: "full",
    data: ledState,
    timestamp: ts(),
  });
}

function handleMockLedCommand(socket, ledState, payload) {
  if (!payload || typeof payload !== "object") {
    return;
  }

  if (payload.type === "relay" && payload.action === "toggle") {
    const current = ledState.relays?.[0]?.state === "ON";
    if (!ledState.relays) ledState.relays = {};
    ledState.relays[0] = { state: current ? "OFF" : "ON" };
    emitLedStatus(socket, ledState);
    return;
  }

  if (payload.type !== "strip" || typeof payload.index !== "number" || !payload.action) {
    return;
  }

  const idx = String(payload.index);
  const strip = ledState.strips?.[idx];
  if (!strip) {
    return;
  }

  if (payload.action === "on") {
    strip.state = "ON";
    if ((strip.brightness ?? 0) <= 0) {
      strip.brightness = 60;
    }
  } else if (payload.action === "off") {
    strip.state = "OFF";
  } else if (payload.action === "toggle") {
    strip.state = strip.state === "ON" ? "OFF" : "ON";
  } else if (payload.action === "brightness" && typeof payload.value === "number") {
    strip.brightness = Math.max(0, Math.min(100, Math.round(payload.value)));
    strip.state = strip.brightness > 0 ? "ON" : "OFF";
  } else if (payload.action === "mode" && typeof payload.value === "string") {
    strip.mode = payload.value.toUpperCase();
    strip.state = strip.mode === "OFF" ? "OFF" : "ON";
  } else if (payload.action === "apply" && payload.payload && typeof payload.payload === "object") {
    Object.assign(strip, payload.payload);
    if (typeof strip.mode === "string") {
      strip.mode = strip.mode.toUpperCase();
    }
    if (strip.state !== "ON" && strip.state !== "OFF") {
      strip.state = "ON";
    }
  }

  emitLedStatus(socket, ledState);
}

function createInitialApplianceState() {
  return JSON.parse(JSON.stringify(STATIC.applianceStatusUpdate.data));
}

function emitApplianceStatus(socket, applianceState) {
  socket.emit("applianceStatusUpdate", {
    type: "full",
    data: applianceState,
    timestamp: ts(),
  });
}

function handleMockApplianceCommand(socket, applianceState, payload) {
  if (!payload || typeof payload !== "object") {
    return;
  }

  if (payload.type !== "relay" || payload.action !== "toggle") {
    return;
  }

  const index = String(payload.index);
  const relay = applianceState.relays?.[index];
  if (!relay) {
    return;
  }

  relay.state = relay.state === "ON" ? "OFF" : "ON";
  emitApplianceStatus(socket, applianceState);
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
  emitVictronStatus(socket, randomVictronPayload());

  // Client hooks may attach after the first burst; resend sensors shortly after connect.
  [150, 600].forEach((ms) => {
    setTimeout(() => {
      socket.emit("sensorUpdate", randomSensorPayload());
      emitVictronStatus(socket, randomVictronPayload());
    }, ms);
  });
}

const server = http.createServer();
const io = new Server(server, {
  cors: { origin: true, credentials: true },
});

io.on("connection", (socket) => {
  console.log(`[mock] client connected ${socket.id}`);
  const ledState = createInitialLedState();
  const applianceState = createInitialApplianceState();

  // Let the browser attach Socket.io listeners before the first burst (React useEffect).
  const connectDelayMs = Number(process.env.MOCK_CONNECT_DELAY_MS || 450);
  let sensorTimer = null;
  const connectTimer = setTimeout(() => {
    sendAll(io, socket);
    const sensorIntervalMs = Number(process.env.MOCK_SENSOR_INTERVAL_MS || 4000);
    sensorTimer = setInterval(() => {
      socket.emit("sensorUpdate", randomSensorPayload());
      emitVictronStatus(socket, randomVictronPayload());
    }, sensorIntervalMs);
  }, connectDelayMs);

  socket.on("ledCommand", (payload) => {
    handleMockLedCommand(socket, ledState, payload);
    if (process.env.DEBUG_MOCK_SOCKET) {
      console.log("[mock] ledCommand", payload);
    }
  });

  socket.on("applianceCommand", (payload) => {
    handleMockApplianceCommand(socket, applianceState, payload);
    if (process.env.DEBUG_MOCK_SOCKET) {
      console.log("[mock] applianceCommand", payload);
    }
  });

  const noop = [
    "floorHeatingCommand",
    "levelingCommand",
    "damperCommand",
    "tableCommand",
    "forceModuleUpdate",
  ];
  for (const ev of noop) {
    socket.on(ev, (payload) => {
      if (ev === "forceModuleUpdate") {
        const moduleId = payload?.moduleId;
        if (moduleId === "module-6") {
          emitVictronStatus(socket, randomVictronPayload());
        } else if (
          moduleId === "module-1" ||
          moduleId === "module-5" ||
          moduleId === "module-7"
        ) {
          socket.emit("sensorUpdate", randomSensorPayload());
        }
      }
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
  console.log(
    `[mock] AC charger: ${AC_CHARGER_CYCLE_MS / 1000}s on / ${AC_CHARGER_CYCLE_MS / 1000}s off (no payload)`
  );
});
