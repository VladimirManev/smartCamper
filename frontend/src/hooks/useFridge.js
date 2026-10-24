/**
 * Fridge BLE status + commands (module-6 Alpicool / AAOBOSI).
 * Power ON/OFF stays on module-5 relay; this is mode + setpoints only.
 *
 * Status updates arrive from ephemeral ESP sessions (~20s poll + after commands).
 * Values older than FRIDGE_STALE_MS are marked stale so the UI can show dashes.
 */

import { useState, useEffect, useCallback, useRef } from "react";

/** No fresh MQTT since this long → treat readings as missing. */
export const FRIDGE_STALE_MS = 45000;

const defaultStatus = {
  connected: false,
  haveStatus: false,
  fresh: false,
  mode: null, // "eco" | "max"
  zone1: { temp: null, setpoint: null },
  zone2: { temp: null, setpoint: null },
  updatedAt: null,
  receivedAt: null,
  ageMs: null,
  lastIntervalMs: null,
};

/**
 * @param {Object} socket - Socket.io instance
 * @returns {{ status, sendFridgeCommand }}
 */
export const useFridge = (socket) => {
  const [status, setStatus] = useState(defaultStatus);
  const lastReceivedAtRef = useRef(null);

  useEffect(() => {
    if (!socket) {
      return;
    }

    const handleUpdate = (data) => {
      if (data?.type !== "full" || !data.data) {
        return;
      }
      const d = data.data;
      const receivedAt =
        typeof d.receivedAt === "number" ? d.receivedAt : Date.now();
      const prev = lastReceivedAtRef.current;
      const lastIntervalMs =
        prev != null && receivedAt > prev ? receivedAt - prev : null;
      lastReceivedAtRef.current = receivedAt;

      if (lastIntervalMs != null) {
        console.debug(
          `[fridge] status interval ${Math.round(lastIntervalMs / 1000)}s`
        );
      }

      const ageMs = Math.max(0, Date.now() - receivedAt);
      const fresh = ageMs <= FRIDGE_STALE_MS;

      setStatus({
        connected: Boolean(d.connected),
        haveStatus: Boolean(d.haveStatus ?? d.connected),
        fresh,
        mode: d.mode === "eco" || d.mode === "max" ? d.mode : null,
        zone1: {
          temp: typeof d.zone1?.temp === "number" ? d.zone1.temp : null,
          setpoint:
            typeof d.zone1?.setpoint === "number" ? d.zone1.setpoint : null,
        },
        zone2: {
          temp: typeof d.zone2?.temp === "number" ? d.zone2.temp : null,
          setpoint:
            typeof d.zone2?.setpoint === "number" ? d.zone2.setpoint : null,
        },
        updatedAt: typeof d.updatedAt === "number" ? d.updatedAt : null,
        receivedAt,
        ageMs,
        lastIntervalMs,
      });
    };

    socket.on("fridgeStatusUpdate", handleUpdate);
    return () => {
      socket.off("fridgeStatusUpdate", handleUpdate);
    };
  }, [socket]);

  // Recompute freshness while values are on screen (no new MQTT).
  useEffect(() => {
    const id = setInterval(() => {
      setStatus((prev) => {
        if (prev.receivedAt == null) {
          return prev;
        }
        const ageMs = Math.max(0, Date.now() - prev.receivedAt);
        const fresh = ageMs <= FRIDGE_STALE_MS;
        if (fresh === prev.fresh && ageMs === prev.ageMs) {
          return prev;
        }
        return { ...prev, ageMs, fresh };
      });
    }, 1000);
    return () => clearInterval(id);
  }, []);

  const sendFridgeCommand = useCallback(
    (command) => {
      if (!socket || !socket.connected) {
        console.warn("⚠️ Cannot send fridge command — socket not connected");
        return;
      }
      socket.emit("fridgeCommand", command);
    },
    [socket]
  );

  return { status, sendFridgeCommand };
};
