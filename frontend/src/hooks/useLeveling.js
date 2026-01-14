/**
 * useLeveling Hook
 * Manages leveling sensor data and commands
 * Sends start command periodically when modal is open
 */

import { useState, useEffect, useRef } from "react";

/**
 * useLeveling hook
 * @param {Object} socket - Socket.io socket instance
 * @param {boolean} isModalOpen - Whether the leveling modal is open
 * @returns {Object} Leveling data and control functions
 */
export const useLeveling = (socket, isModalOpen) => {
  const [pitch, setPitch] = useState(null);
  const [roll, setRoll] = useState(null);
  const intervalRef = useRef(null);

  // Send start command periodically when modal is open
  useEffect(() => {
    if (!socket || !isModalOpen) {
      // Clear interval if modal is closed
      if (intervalRef.current) {
        clearInterval(intervalRef.current);
        intervalRef.current = null;
      }
      return;
    }

    // Send initial start command immediately
    socket.emit("levelingCommand", { type: "start" });

    // Send start command every 10 seconds
    intervalRef.current = setInterval(() => {
      socket.emit("levelingCommand", { type: "start" });
    }, 10000); // 10 seconds

    // Cleanup on unmount or when modal closes
    return () => {
      if (intervalRef.current) {
        clearInterval(intervalRef.current);
        intervalRef.current = null;
      }
    };
  }, [socket, isModalOpen]);

  // Listen for leveling data
  useEffect(() => {
    if (!socket) return;

    const handleLevelingData = (data) => {
      if (data.pitch !== undefined && data.roll !== undefined) {
        setPitch(data.pitch);
        setRoll(data.roll);
      }
    };

    socket.on("levelingData", handleLevelingData);

    return () => {
      socket.off("levelingData", handleLevelingData);
    };
  }, [socket]);

  return {
    pitch,
    roll,
  };
};
