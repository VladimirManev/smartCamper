/**
 * useSecurityController Hook
 * Module-8 security status (zone 1 alarm + zone 2 perimeter) via Socket.io.
 */

import { useCallback, useEffect, useRef, useState } from "react";

const EMPTY_PERIMETER_MOTION = {};

/**
 * Merge mock/backend timestamps with rising edges from inputs.perimeter bools
 * (ESP publishes bools; mock also sends perimeterLastMotion).
 */
function mergePerimeterLastMotion(prev, inputs, armed, prevLevels) {
  const levels = inputs?.perimeter;
  if (levels && typeof levels === "object") {
    // Always track levels so arming with PIR already HIGH is not a fake rising edge
    for (const [id, on] of Object.entries(levels)) {
      if (!armed) {
        prevLevels[id] = !!on;
      }
    }
  }

  if (!armed) {
    return EMPTY_PERIMETER_MOTION;
  }

  const next = { ...prev };
  const now = Date.now();
  const fromServer = inputs?.perimeterLastMotion;

  if (fromServer && typeof fromServer === "object") {
    for (const [id, ts] of Object.entries(fromServer)) {
      if (ts != null) {
        next[id] = ts;
      }
    }
  }

  if (levels && typeof levels === "object") {
    for (const [id, on] of Object.entries(levels)) {
      const active = !!on;
      if (active && !prevLevels[id]) {
        next[id] = now;
      }
      prevLevels[id] = active;
    }
  }

  return next;
}

/**
 * @param {Object} socket - Socket.io instance
 * @returns {Object}
 */
export function useSecurityController(socket) {
  const [zone1Armed, setZone1ArmedState] = useState(false);
  const [zone1Phase, setZone1Phase] = useState("idle");
  const [zone1CatMode, setZone1CatMode] = useState(false);
  const [zone1ExitStartedAt, setZone1ExitStartedAt] = useState(null);
  const [zone1ForceKeypad, setZone1ForceKeypad] = useState(false);
  const [zone2Armed, setZone2ArmedState] = useState(false);
  const [perimeterLastMotion, setPerimeterLastMotion] = useState(
    EMPTY_PERIMETER_MOTION
  );
  const [doors, setDoors] = useState({
    driver: false,
    passenger: false,
    sliding: false,
    rear: false,
  });

  const zone1PhaseRef = useRef("idle");
  const prevPerimeterLevelsRef = useRef({});

  const applyZone1Phase = useCallback((phase) => {
    const prevPhase = zone1PhaseRef.current;
    if (phase === prevPhase) {
      return;
    }
    zone1PhaseRef.current = phase;
    setZone1Phase(phase);

    if (phase === "exit_delay") {
      setZone1ExitStartedAt(Date.now());
      setZone1ForceKeypad(false);
    } else {
      setZone1ExitStartedAt(null);
      setZone1ForceKeypad(false);
    }
  }, []);

  useEffect(() => {
    if (!socket) {
      return undefined;
    }

    const handleSecurityStatusUpdate = (data) => {
      if (data?.type !== "full" || !data.data) {
        return;
      }

      const { zone1, zone2, inputs } = data.data;

      const z1Armed = !!zone1?.armed;
      const phase = zone1?.phase || (z1Armed ? "armed" : "idle");
      setZone1ArmedState(z1Armed);
      applyZone1Phase(phase);
      setZone1CatMode(z1Armed && !!zone1?.ignoreInteriorPir);

      const armed = !!zone2?.armed;
      setZone2ArmedState(armed);

      setPerimeterLastMotion((prev) =>
        mergePerimeterLastMotion(
          prev,
          inputs,
          armed,
          prevPerimeterLevelsRef.current
        )
      );

      if (inputs?.doors) {
        setDoors({
          driver: !!inputs.doors.driver,
          passenger: !!inputs.doors.passenger,
          sliding: !!inputs.doors.sliding,
          rear: !!inputs.doors.rear,
        });
      }
    };

    socket.on("securityStatusUpdate", handleSecurityStatusUpdate);

    return () => {
      socket.off("securityStatusUpdate", handleSecurityStatusUpdate);
    };
  }, [socket, applyZone1Phase]);

  const sendSecurityCommand = useCallback(
    (command) => {
      if (!socket?.connected) {
        console.warn("⚠️ Cannot send security command - socket not connected");
        return;
      }
      socket.emit("securityCommand", command);
    },
    [socket]
  );

  const armZone1 = useCallback(
    (catMode = false) => {
      setZone1ArmedState(true);
      setZone1CatMode(!!catMode);
      applyZone1Phase("exit_delay");
      sendSecurityCommand({
        zone: 1,
        action: "on",
        ignoreInteriorPir: !!catMode,
      });
    },
    [applyZone1Phase, sendSecurityCommand]
  );

  const disarmZone1 = useCallback(
    (pin) => {
      sendSecurityCommand({ zone: 1, action: "off", pin: String(pin || "") });
    },
    [sendSecurityCommand]
  );

  const forceZone1Keypad = useCallback(() => {
    setZone1ForceKeypad(true);
  }, []);

  const setZone2Armed = useCallback(
    (armed) => {
      setZone2ArmedState(armed);
      if (!armed) {
        setPerimeterLastMotion(EMPTY_PERIMETER_MOTION);
      }
      sendSecurityCommand({ zone: 2, action: armed ? "on" : "off" });
    },
    [sendSecurityCommand]
  );

  const simulatePerimeterMotion = useCallback(
    (sensorId) => {
      if (!zone2Armed) {
        return;
      }
      setPerimeterLastMotion((prev) => ({
        ...prev,
        [sensorId]: Date.now(),
      }));
      sendSecurityCommand({
        zone: 2,
        action: "simulate_motion",
        sensor: sensorId,
      });
    },
    [sendSecurityCommand, zone2Armed]
  );

  const simulateZone1Trip = useCallback(() => {
    sendSecurityCommand({ zone: 1, action: "simulate_trip" });
  }, [sendSecurityCommand]);

  return {
    zone1Armed,
    zone1Phase,
    zone1CatMode,
    zone1ExitStartedAt,
    zone1ForceKeypad,
    forceZone1Keypad,
    armZone1,
    disarmZone1,
    zone2Armed,
    perimeterLastMotion,
    doors,
    setZone2Armed,
    simulatePerimeterMotion,
    simulateZone1Trip,
  };
}
