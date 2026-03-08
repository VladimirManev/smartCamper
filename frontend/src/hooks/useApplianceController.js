/**
 * useApplianceController Hook
 * Manages appliance relays state (Audio System, Water Pump, Refrigerator)
 * Handles appliance commands to backend
 */

import { useState, useEffect } from "react";

/**
 * Custom hook for appliance controller
 * @param {Object} socket - Socket.io instance
 * @returns {Object} { appliances, sendApplianceCommand }
 */
export const useApplianceController = (socket) => {
  const [appliances, setAppliances] = useState({
    0: { state: "OFF" }, // Audio System
    1: { state: "OFF" }, // Water Pump
    2: { state: "OFF" }, // Refrigerator
    3: { state: "OFF" }, // WC Fan
    4: { state: "OFF" }, // Boiler
    5: { state: "OFF" }, // Inverter
  });

  useEffect(() => {
    if (!socket) {
      return;
    }

    // Listen for appliance status updates
    const handleApplianceStatusUpdate = (data) => {
      console.log("🔌 Appliance Status Update:", data);

      // Full status format
      if (data.type === "full" && data.data) {
        const statusData = data.data;

        // Update all relays
        if (statusData.relays) {
          const newAppliances = {};
          for (const [index, relayData] of Object.entries(statusData.relays)) {
            newAppliances[index] = {
              state: relayData.state,
            };
          }
          setAppliances((prev) => ({ ...prev, ...newAppliances }));
        }
      }
    };

    socket.on("applianceStatusUpdate", handleApplianceStatusUpdate);

    // Cleanup
    return () => {
      socket.off("applianceStatusUpdate", handleApplianceStatusUpdate);
    };
  }, [socket]);

  /**
   * Send appliance command to backend
   * @param {Object} command - Command object { type, index, action }
   */
  const sendApplianceCommand = (command) => {
    if (!socket || !socket.connected) {
      console.warn("⚠️ Cannot send appliance command - socket not connected");
      return;
    }

    socket.emit("applianceCommand", command);
  };

  return {
    appliances,
    sendApplianceCommand,
  };
};
