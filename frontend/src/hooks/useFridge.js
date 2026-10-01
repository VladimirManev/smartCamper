/**
 * Fridge BLE status + commands (module-6 Alpicool / AAOBOSI).
 * Power ON/OFF stays on module-5 relay; this is mode + setpoints only.
 */

import { useState, useEffect, useCallback } from "react";

const defaultStatus = {
  connected: false,
  mode: null, // "eco" | "max"
  zone1: { temp: null, setpoint: null },
  zone2: { temp: null, setpoint: null },
  updatedAt: null,
};

/**
 * @param {Object} socket - Socket.io instance
 * @returns {{ status, sendFridgeCommand }}
 */
export const useFridge = (socket) => {
  const [status, setStatus] = useState(defaultStatus);

  useEffect(() => {
    if (!socket) {
      return;
    }

    const handleUpdate = (data) => {
      if (data?.type !== "full" || !data.data) {
        return;
      }
      const d = data.data;
      setStatus({
        connected: Boolean(d.connected),
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
      });
    };

    socket.on("fridgeStatusUpdate", handleUpdate);
    return () => {
      socket.off("fridgeStatusUpdate", handleUpdate);
    };
  }, [socket]);

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
