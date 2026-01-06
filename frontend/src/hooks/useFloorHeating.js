/**
 * useFloorHeating Hook
 * Manages floor heating circles state
 * Handles floor heating commands to backend
 */

import { useState, useEffect } from "react";

/**
 * Custom hook for floor heating controller
 * @param {Object} socket - Socket.io instance
 * @returns {Object} { circles, sendFloorHeatingCommand }
 */
export const useFloorHeating = (socket) => {
  const [circles, setCircles] = useState({
    0: { mode: "OFF", relay: "OFF", temperature: null, error: false }, // Central 1
    1: { mode: "OFF", relay: "OFF", temperature: null, error: false }, // Central 2
    2: { mode: "OFF", relay: "OFF", temperature: null, error: false }, // Bathroom
    3: { mode: "OFF", relay: "OFF", temperature: null, error: false }, // Podium
  });

  useEffect(() => {
    if (!socket) {
      return;
    }

    // Listen for floor heating status updates
    const handleFloorHeatingStatusUpdate = (data) => {
      console.log("🔥 Floor Heating Status Update:", data);

      // NEW FORMAT: Full status in one object
      if (data.type === "full" && data.data) {
        const statusData = data.data;

        // Update all circles
        if (statusData.circles) {
          const newCircles = {};
          for (const [index, circleData] of Object.entries(statusData.circles)) {
            newCircles[index] = {
              mode: circleData.mode || "OFF",
              relay: circleData.relay || "OFF",
              temperature: circleData.temperature !== null && circleData.temperature !== undefined ? circleData.temperature : null,
              error: circleData.error || false,
            };
          }
          setCircles((prev) => ({ ...prev, ...newCircles }));
        }
      }
      // SINGLE CIRCLE FORMAT
      else if (data.type === "circle" && typeof data.index === "number") {
        // Always update with server state (overwrites optimistic update)
        setCircles((prev) => ({
          ...prev,
          [data.index]: {
            mode: data.mode || "OFF",
            relay: data.relay || "OFF",
            temperature: data.temperature !== null && data.temperature !== undefined ? data.temperature : null,
            error: data.error || false,
          },
        }));
      }
      // ERROR FORMAT
      else if (data.type === "error" && typeof data.index === "number") {
        setCircles((prev) => ({
          ...prev,
          [data.index]: {
            ...prev[data.index],
            error: true,
          },
        }));
      }
    };

    socket.on("floorHeatingStatusUpdate", handleFloorHeatingStatusUpdate);

    // Cleanup
    return () => {
      socket.off("floorHeatingStatusUpdate", handleFloorHeatingStatusUpdate);
    };
  }, [socket]);

  /**
   * Send floor heating command to backend
   * @param {Object} command - Command object { type, index?, action, value? }
   */
  const sendFloorHeatingCommand = (command) => {
    if (!socket || !socket.connected) {
      console.warn("⚠️ Cannot send floor heating command - socket not connected");
      return;
    }

    console.log("🔥 Sending floor heating command:", command);
    socket.emit("floorHeatingCommand", command);
  };

  return {
    circles,
    sendFloorHeatingCommand,
  };
};

