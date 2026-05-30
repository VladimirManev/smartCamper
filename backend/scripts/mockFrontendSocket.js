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
    batteryLevel: 68,
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
    batteryLevel: Math.min(98, Math.max(8, Math.round(55 + Math.sin(t / 31) * 40))),
    timestamp: ts(),
  };
}

function buildWireAmps(nodes) {
  const pv = (power, voltage) =>
    power && voltage > 0 ? Math.round((power / voltage) * 10) / 10 : 0;

  return {
    "solar1-mppt1": pv(nodes.solarPanelGroup1?.power, nodes.solarPanelGroup1?.voltage),
    "solar2-mppt2": pv(nodes.solarPanelGroup2?.power, nodes.solarPanelGroup2?.voltage),
    "alternator-dcdc": nodes.alternator?.current ?? 0,
    "dcdc-battery": nodes.dcDcBooster?.current ?? 0,
    "mppt1-battery": pv(nodes.solarController1?.power, nodes.solarController1?.voltage),
    "mppt2-battery": pv(nodes.solarController2?.power, nodes.solarController2?.voltage),
    "loads-battery": pv(nodes.dcLoads?.power, nodes.dcLoads?.voltage),
    "ac-battery": pv(
      (nodes.charger230v?.voltage ?? 230) * (nodes.charger230v?.current ?? 0),
      12.6
    ),
  };
}

function buildBatteryFlow(wireAmps, batteryVoltage = 12.6) {
  const chargeIn = ["dcdc-battery", "mppt1-battery", "mppt2-battery", "ac-battery"].reduce(
    (sum, id) => sum + (Number(wireAmps[id]) || 0),
    0
  );
  const dischargeOut = Number(wireAmps["loads-battery"]) || 0;
  const netAmps = Math.round((chargeIn - dischargeOut) * 10) / 10;
  const absAmps = Math.round(Math.abs(netAmps) * 10) / 10;
  const watts = Math.round(absAmps * batteryVoltage);

  let direction = "idle";
  if (netAmps > 0.05) direction = "charge";
  else if (netAmps < -0.05) direction = "discharge";

  return { direction, amps: absAmps, watts, netAmps };
}

function randomBatterySystemPayload() {
  const t = Date.now() / 1000;
  const batteryLevel = Math.min(
    98,
    Math.max(8, Math.round(55 + Math.sin(t / 31) * 40))
  );

  // Rotate scenarios every ~18s to demo zero-current paths and net battery states.
  const scenario = Math.floor(t / 18) % 5;
  const sun = 0.55 + 0.45 * Math.max(0, Math.sin(t / 25));

  let solar1Power = Math.round(320 * sun + Math.sin(t / 5) * 20);
  let solar2Power = Math.round(290 * sun + Math.cos(t / 6) * 15);
  let alternatorCurrent = Math.round((18 + Math.sin(t / 6) * 12) * 10) / 10;
  let dcDcCurrent = Math.round((12 + Math.sin(t / 7) * 8) * 10) / 10;
  let acCurrent = Math.round((2.2 + Math.sin(t / 11) * 1.4) * 10) / 10;
  let loadsPower = Math.round(120 + 80 * (1 - batteryLevel / 100) + Math.sin(t / 4) * 30);

  switch (scenario) {
    case 0:
      // Night — no solar, no alternator, no AC; loads only.
      solar1Power = 0;
      solar2Power = 0;
      alternatorCurrent = 0;
      dcDcCurrent = 0;
      acCurrent = 0;
      break;
    case 1:
      // Solar only — panels + MPPT, no engine or shore power.
      alternatorCurrent = 0;
      dcDcCurrent = 0;
      acCurrent = 0;
      break;
    case 2:
      // Heavy discharge — loads on, all chargers off.
      solar1Power = 0;
      solar2Power = 0;
      alternatorCurrent = 0;
      dcDcCurrent = 0;
      acCurrent = 0;
      loadsPower = Math.round(260 + Math.sin(t / 4) * 35);
      break;
    case 3:
      // Full charge — all sources, no loads.
      loadsPower = 0;
      break;
    case 4:
    default:
      // Mixed — individual paths drop to zero over time.
      if (Math.sin(t / 9) < 0) solar2Power = 0;
      if (Math.cos(t / 11) < 0) {
        alternatorCurrent = 0;
        dcDcCurrent = 0;
      }
      if (Math.sin(t / 13) < -0.2) acCurrent = 0;
      if (Math.cos(t / 7) < 0) loadsPower = 0;
      break;
  }

  if (alternatorCurrent <= 0) dcDcCurrent = 0;

  const solar1Voltage = Math.round((19 + Math.sin(t / 12) * 0.6) * 10) / 10;
  const solar2Voltage = Math.round((18.7 + Math.cos(t / 13) * 0.5) * 10) / 10;

  const nodes = {
      charger230v: {
        voltage: 230,
        current: Math.max(0, acCurrent),
      },
      dcDcBooster: {
        voltage: Math.round((14.1 + Math.sin(t / 9) * 0.3) * 10) / 10,
        current: Math.max(0, dcDcCurrent),
      },
      alternator: {
        voltage: Math.round((13.8 + Math.sin(t / 8) * 0.4) * 10) / 10,
        current: Math.max(0, alternatorCurrent),
      },
      solarController1: {
        power: Math.max(0, solar1Power),
        voltage: solar1Voltage,
      },
      solarController2: {
        power: Math.max(0, solar2Power),
        voltage: solar2Voltage,
      },
      solarPanelGroup1: {
        power: Math.max(0, solar1Power),
        voltage: solar1Voltage,
      },
      solarPanelGroup2: {
        power: Math.max(0, solar2Power),
        voltage: solar2Voltage,
      },
      dcLoads: {
        power: Math.max(0, loadsPower),
        voltage: Math.round((12.6 + Math.sin(t / 15) * 0.4) * 10) / 10,
      },
    };
  const wireAmps = buildWireAmps(nodes);
  return {
    batteryLevel,
    nodes,
    wireAmps,
    batteryFlow: buildBatteryFlow(wireAmps),
    timestamp: ts(),
  };
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
  socket.emit("batterySystemUpdate", randomBatterySystemPayload());
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
      socket.emit("batterySystemUpdate", randomBatterySystemPayload());
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
      socket.emit("batterySystemUpdate", randomBatterySystemPayload());
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
