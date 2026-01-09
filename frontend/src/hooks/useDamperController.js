/**
 * useDamperController Hook
 * Manages damper (air vent) state
 * Handles damper commands to backend
 */

import { useState, useEffect } from "react";

/**
 * Custom hook for damper controller
 * @param {Object} socket - Socket.io instance
 * @returns {Object} { dampers, sendDamperCommand }
 */
export const useDamperController = (socket) => {
  // Initialize all 5 dampers (0-4) with default angle 90° (open)
  const [dampers, setDampers] = useState({
    0: { angle: 90 }, // Damper 0 - Front
    1: { angle: 90 }, // Damper 1 - Rear
    2: { angle: 90 }, // Damper 2 - Bath
    3: { angle: 90 }, // Damper 3 - Shoes
    4: { angle: 90 }, // Damper 4 - Cockpit
  });

  useEffect(() => {
    if (!socket) {
      return;
    }

    // Listen for damper status updates
    const handleDamperStatusUpdate = (data) => {
      console.log("🌬️ Damper Status Update:", data);

      if (data.type === "damper" && typeof data.index === "number") {
        setDampers((prev) => ({
          ...prev,
          [data.index]: {
            angle: data.angle,
          },
        }));
      }
    };

    socket.on("damperStatusUpdate", handleDamperStatusUpdate);

    // Cleanup
    return () => {
      socket.off("damperStatusUpdate", handleDamperStatusUpdate);
    };
  }, [socket]);

  /**
   * Send damper command to backend
   * @param {Object} command - Command object { type, index, action, angle }
   */
  const sendDamperCommand = (command) => {
    if (!socket || !socket.connected) {
      console.warn("⚠️ Cannot send damper command - socket not connected");
      return;
    }

    socket.emit("damperCommand", command);
  };

  return {
    dampers,
    sendDamperCommand,
  };
};

