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
const PERIMETER_SENSORS = [
  "front",
  "rear",
  "left_front",
  "left_rear",
  "right_front",
  "right_rear",
];
let lastLoggedAcPhase = null;

function isMockAcChargerLive() {
  return Math.floor(Date.now() / AC_CHARGER_CYCLE_MS) % 2 === 0;
}

function isVictronDeviceStale(publishedAt, updatedAt) {
  if (publishedAt == null || updatedAt == null) return true;
  return publishedAt - updatedAt > VICTRON_STALE_MS;
}

/** Refresh publishedAt/updatedAt so frontend stale checks stay LIVE. */
function withFreshVictronTimestamps(data) {
  const publishedAt = Date.now();
  const updatedAt = publishedAt - 200;
  const copy = JSON.parse(JSON.stringify(data));
  copy.publishedAt = publishedAt;
  for (const [key, device] of Object.entries(copy)) {
    if (key === "publishedAt") continue;
    if (device && typeof device === "object" && "updatedAt" in device) {
      device.updatedAt = updatedAt;
    }
  }
  return copy;
}

function buildStaticVictronPayload() {
  return withFreshVictronTimestamps(STATIC.victronStatusUpdate.data);
}

function ts() {
  return new Date().toISOString();
}

function buildModuleStatuses() {
  const modules = {};
  for (let i = 1; i <= 8; i++) {
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
        6: { state: "ON" },
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
  const publishedAt = Date.now();
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
  if (process.env.DEBUG_MOCK_VICTRON) {
    const shunt = data?.smartshunt;
    console.log(
      `[mock] victron smartshunt ${shunt?.voltage ?? "—"}V ${shunt?.current ?? "—"}A ${shunt?.soc ?? "—"}%`
    );
  }
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

function createInitialSecurityState() {
  return {
    zone1: {
      armed: false,
      phase: "idle",
      ignoreInteriorPir: false,
      siren: false,
      smoke: false,
    },
    zone2: { armed: false },
    inputs: {
      spareOpen: false,
      interiorPir: false,
      doors: {
        driver: false,
        passenger: false,
        sliding: false,
        rear: false,
      },
      perimeter: Object.fromEntries(
        PERIMETER_SENSORS.map((sensor) => [sensor, false])
      ),
      perimeterLastMotion: Object.fromEntries(
        PERIMETER_SENSORS.map((sensor) => [sensor, null])
      ),
    },
  };
}

function emitSecurityStatus(socket, securityState) {
  socket.emit("securityStatusUpdate", {
    type: "full",
    data: JSON.parse(JSON.stringify(securityState)),
    timestamp: ts(),
  });
}

function clearPerimeterMotion(securityState) {
  for (const sensor of PERIMETER_SENSORS) {
    securityState.inputs.perimeter[sensor] = false;
    securityState.inputs.perimeterLastMotion[sensor] = null;
  }
}

function simulatePerimeterMotion(securityState, sensorId) {
  const sensor = PERIMETER_SENSORS.includes(sensorId)
    ? sensorId
    : PERIMETER_SENSORS[Math.floor(Math.random() * PERIMETER_SENSORS.length)];
  const now = Date.now();
  securityState.inputs.perimeter[sensor] = true;
  securityState.inputs.perimeterLastMotion[sensor] = now;
  return sensor;
}

const MOCK_ALARM_PIN = process.env.MOCK_ALARM_PIN || "1234";
const MOCK_EXIT_DELAY_MS = Number(process.env.MOCK_EXIT_DELAY_MS || 30000);
const MOCK_ENTRY_DELAY_MS = Number(process.env.MOCK_ENTRY_DELAY_MS || 30000);

function createZone1DelayController(socket, securityState) {
  let timer = null;

  function clearTimer() {
    if (timer) {
      clearTimeout(timer);
      timer = null;
    }
  }

  function resetIdle() {
    clearTimer();
    securityState.zone1.armed = false;
    securityState.zone1.phase = "idle";
    securityState.zone1.ignoreInteriorPir = false;
    securityState.zone1.siren = false;
    securityState.zone1.smoke = false;
  }

  return {
    clearTimer,
    arm(ignoreInteriorPir) {
      clearTimer();
      securityState.zone1.armed = true;
      securityState.zone1.phase = "exit_delay";
      securityState.zone1.ignoreInteriorPir = !!ignoreInteriorPir;
      securityState.zone1.siren = false;
      securityState.zone1.smoke = false;
      emitSecurityStatus(socket, securityState);

      timer = setTimeout(() => {
        timer = null;
        if (securityState.zone1.phase !== "exit_delay") return;
        securityState.zone1.phase = "armed";
        emitSecurityStatus(socket, securityState);
      }, MOCK_EXIT_DELAY_MS);
    },
    disarm(pin) {
      if (pin !== MOCK_ALARM_PIN) {
        if (process.env.DEBUG_MOCK_SOCKET) {
          console.log("[mock] zone1 disarm rejected (bad PIN)");
        }
        // Re-emit current status so UI can clear and retry
        emitSecurityStatus(socket, securityState);
        return false;
      }
      resetIdle();
      emitSecurityStatus(socket, securityState);
      return true;
    },
    /** Bench: trip while armed → entry_delay → alarm */
    trip() {
      if (securityState.zone1.phase !== "armed") return;
      clearTimer();
      securityState.zone1.phase = "entry_delay";
      emitSecurityStatus(socket, securityState);
      timer = setTimeout(() => {
        timer = null;
        if (securityState.zone1.phase !== "entry_delay") return;
        securityState.zone1.phase = "alarm";
        securityState.zone1.siren = true;
        emitSecurityStatus(socket, securityState);
      }, MOCK_ENTRY_DELAY_MS);
    },
    stop() {
      clearTimer();
    },
  };
}

function handleMockSecurityCommand(socket, securityState, zone1Ctrl, payload) {
  if (!payload || typeof payload !== "object") {
    return;
  }

  if (payload.zone === 1 && payload.action === "on") {
    zone1Ctrl.arm(payload.ignoreInteriorPir);
    return;
  }

  if (payload.zone === 1 && payload.action === "off") {
    zone1Ctrl.disarm(String(payload.pin || ""));
    return;
  }

  if (payload.zone === 1 && payload.action === "simulate_trip") {
    zone1Ctrl.trip();
    return;
  }

  if (payload.zone === 2 && payload.action === "on") {
    securityState.zone2.armed = true;
    emitSecurityStatus(socket, securityState);
    return;
  }

  if (payload.zone === 2 && payload.action === "off") {
    securityState.zone2.armed = false;
    clearPerimeterMotion(securityState);
    emitSecurityStatus(socket, securityState);
    return;
  }

  if (payload.zone === 2 && payload.action === "simulate_motion") {
    simulatePerimeterMotion(securityState, payload.sensor);
    emitSecurityStatus(socket, securityState);
  }
}

/** Cycle door combos for alarm door-marker tests */
function createDoorSimulator(socket, securityState) {
  const stepMs = Number(process.env.MOCK_DOOR_STEP_MS || 8000);
  const SCENES = [
    {
      driver: false,
      passenger: false,
      sliding: false,
      rear: false,
      label: "all closed",
    },
    {
      driver: true,
      passenger: false,
      sliding: false,
      rear: false,
      label: "driver",
    },
    {
      driver: false,
      passenger: true,
      sliding: false,
      rear: false,
      label: "passenger",
    },
    {
      driver: true,
      passenger: true,
      sliding: false,
      rear: false,
      label: "both front",
    },
    {
      driver: false,
      passenger: false,
      sliding: true,
      rear: false,
      label: "sliding",
    },
    {
      driver: false,
      passenger: false,
      sliding: false,
      rear: true,
      label: "rear",
    },
    {
      driver: true,
      passenger: false,
      sliding: true,
      rear: false,
      label: "driver + sliding",
    },
    {
      driver: false,
      passenger: true,
      sliding: false,
      rear: true,
      label: "passenger + rear",
    },
    {
      driver: true,
      passenger: true,
      sliding: true,
      rear: true,
      label: "all open",
    },
  ];
  let timer = null;
  let index = 0;

  function applyScene(scene) {
    securityState.inputs.doors.driver = !!scene.driver;
    securityState.inputs.doors.passenger = !!scene.passenger;
    securityState.inputs.doors.sliding = !!scene.sliding;
    securityState.inputs.doors.rear = !!scene.rear;
    console.log(`[mock] doors: ${scene.label}`);
    emitSecurityStatus(socket, securityState);
  }

  function tick() {
    index = (index + 1) % SCENES.length;
    applyScene(SCENES[index]);
    timer = setTimeout(tick, stepMs);
  }

  return {
    start() {
      if (timer) return;
      applyScene(SCENES[0]);
      timer = setTimeout(tick, stepMs);
    },
    stop() {
      if (timer) {
        clearTimeout(timer);
        timer = null;
      }
    },
  };
}

function createRoundMinuteMotionSimulator(socket, securityState, zone1Ctrl) {
  let timeout = null;
  let interval = null;

  function fireMotion() {
    const now = new Date();
    const sensor = simulatePerimeterMotion(securityState);
    securityState.inputs.interiorPir = true;

    console.log(
      `[mock] :${String(now.getMinutes()).padStart(2, "0")} motion (perimeter=${sensor}, zone1=${securityState.zone1.phase})`
    );

    if (securityState.zone1.phase === "armed") {
      zone1Ctrl.trip();
    } else {
      emitSecurityStatus(socket, securityState);
    }

    setTimeout(() => {
      if (securityState.inputs.interiorPir) {
        securityState.inputs.interiorPir = false;
        emitSecurityStatus(socket, securityState);
      }
    }, 800);
  }

  function msUntilNextRoundMinute() {
    const now = Date.now();
    return 60_000 - (now % 60_000);
  }

  return {
    start() {
      if (timeout || interval) return;
      const waitMs = msUntilNextRoundMinute();
      console.log(
        `[mock] Next motion pulse in ${Math.ceil(waitMs / 1000)}s (every round minute)`
      );
      timeout = setTimeout(() => {
        timeout = null;
        fireMotion();
        interval = setInterval(fireMotion, 60_000);
      }, waitMs);
    },
    stop() {
      if (timeout) {
        clearTimeout(timeout);
        timeout = null;
      }
      if (interval) {
        clearInterval(interval);
        interval = null;
      }
    },
  };
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

  if (payload.type !== "relay") {
    return;
  }

  const relayActions = ["toggle", "on", "off"];
  if (!relayActions.includes(payload.action)) {
    return;
  }

  const index = String(payload.index);
  const relay = applianceState.relays?.[index];
  if (!relay) {
    return;
  }

  if (payload.action === "toggle") {
    relay.state = relay.state === "ON" ? "OFF" : "ON";
  } else if (payload.action === "on") {
    relay.state = "ON";
  } else if (payload.action === "off") {
    relay.state = "OFF";
  }
  emitApplianceStatus(socket, applianceState);
}

function sendAll(socket, securityState) {
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
  emitVictronStatus(socket, buildStaticVictronPayload());
  emitSecurityStatus(socket, securityState);

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
  const securityState = createInitialSecurityState();
  const zone1Ctrl = createZone1DelayController(socket, securityState);
  const motionSim = createRoundMinuteMotionSimulator(
    socket,
    securityState,
    zone1Ctrl
  );
  const doorSim = createDoorSimulator(socket, securityState);

  // Let the browser attach Socket.io listeners before the first burst (React useEffect).
  const connectDelayMs = Number(process.env.MOCK_CONNECT_DELAY_MS || 450);
  let sensorTimer = null;
  const connectTimer = setTimeout(() => {
    sendAll(socket, securityState);
    motionSim.start();
    doorSim.start();
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

  socket.on("securityCommand", (payload) => {
    handleMockSecurityCommand(socket, securityState, zone1Ctrl, payload);
    if (process.env.DEBUG_MOCK_SOCKET) {
      console.log("[mock] securityCommand", payload);
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
    motionSim.stop();
    doorSim.stop();
    zone1Ctrl.stop();
    console.log(`[mock] client disconnected ${socket.id} (${reason})`);
  });
});

server.listen(PORT, () => {
  console.log(`[mock] Socket.io listening on http://localhost:${PORT}`);
  console.log(`[mock] Alarm PIN: ${MOCK_ALARM_PIN}`);
  console.log(`[mock] Exit/entry delay: ${MOCK_EXIT_DELAY_MS / 1000}s / ${MOCK_ENTRY_DELAY_MS / 1000}s`);
  console.log(`[mock] Motion pulse on every round minute (:00)`);
  console.log(
    `[mock] Front door scenes every ${Number(process.env.MOCK_DOOR_STEP_MS || 8000) / 1000}s (closed → left → right → both)`
  );
  console.log(`[mock] Optional: MOCK_SENSOR_INTERVAL_MS (default 4000)`);
  console.log(
    `[mock] AC charger: ${AC_CHARGER_CYCLE_MS / 1000}s on / ${AC_CHARGER_CYCLE_MS / 1000}s off (no payload)`
  );
});
