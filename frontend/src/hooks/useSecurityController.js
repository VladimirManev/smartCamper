/**
 * useSecurityController Hook
 * Module-8 security status (zone 2 perimeter) via Socket.io.
 */

import { useCallback, useEffect, useState } from "react";

const EMPTY_PERIMETER_MOTION = {};

/**
 * @param {Object} socket - Socket.io instance
 * @returns {Object}
 */
export function useSecurityController(socket) {
  const [zone2Armed, setZone2ArmedState] = useState(false);
  const [perimeterLastMotion, setPerimeterLastMotion] = useState(
    EMPTY_PERIMETER_MOTION
  );

  useEffect(() => {
    if (!socket) {
      return undefined;
    }

    const handleSecurityStatusUpdate = (data) => {
      if (data?.type !== "full" || !data.data) {
        return;
      }

      const { zone2, inputs } = data.data;
      const armed = !!zone2?.armed;
      setZone2ArmedState(armed);

      if (inputs?.perimeterLastMotion) {
        setPerimeterLastMotion({ ...inputs.perimeterLastMotion });
      } else if (!armed) {
        setPerimeterLastMotion(EMPTY_PERIMETER_MOTION);
      }
    };

    socket.on("securityStatusUpdate", handleSecurityStatusUpdate);

    return () => {
      socket.off("securityStatusUpdate", handleSecurityStatusUpdate);
    };
  }, [socket]);

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

  return {
    zone2Armed,
    perimeterLastMotion,
    setZone2Armed,
    simulatePerimeterMotion,
  };
}
