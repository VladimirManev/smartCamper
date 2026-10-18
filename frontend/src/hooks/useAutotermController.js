/**
 * useAutotermController
 * Autoterm Air 2D status + by-power commands via WebSocket
 */

import { useState, useEffect, useCallback } from "react";

const DEFAULT_STATUS = {
  state: "unknown",
  mode: "unknown",
  power: 5,
  errorCode: 0,
  error: null,
  heaterTemp: null,
  battery: null,
  controlLocked: false,
  lockReason: null,
};

/**
 * @param {Object} socket - Socket.io instance
 */
export const useAutotermController = (socket) => {
  const [status, setStatus] = useState(DEFAULT_STATUS);

  useEffect(() => {
    if (!socket) {
      return;
    }

    const handleUpdate = (data) => {
      if (!data || data.type !== "autoterm") {
        return;
      }
      setStatus({
        state: data.state || "unknown",
        mode: data.mode || "unknown",
        power:
          typeof data.power === "number" && data.power >= 1 && data.power <= 9
            ? data.power
            : 5,
        errorCode:
          typeof data.errorCode === "number" ? data.errorCode : 0,
        error: typeof data.error === "string" ? data.error : null,
        heaterTemp:
          typeof data.heaterTemp === "number" ? data.heaterTemp : null,
        battery: typeof data.battery === "number" ? data.battery : null,
        controlLocked: !!data.controlLocked,
        lockReason: data.lockReason || null,
      });
    };

    socket.on("autotermStatusUpdate", handleUpdate);
    return () => {
      socket.off("autotermStatusUpdate", handleUpdate);
    };
  }, [socket]);

  const sendAutotermCommand = useCallback(
    (command) => {
      if (!socket || !socket.connected) {
        console.warn("⚠️ Cannot send autoterm command - socket not connected");
        return;
      }
      socket.emit("autotermCommand", { type: "autoterm", ...command });
    },
    [socket]
  );

  return { status, sendAutotermCommand };
};
