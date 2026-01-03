/**
 * useLEDController Hook
 * Manages LED strips and relays state
 * Handles LED commands to backend
 */

import { useState, useEffect } from "react";

/**
 * Custom hook for LED controller
 * @param {Object} socket - Socket.io instance
 * @returns {Object} { ledStrips, relays, sendLEDCommand }
 */
export const useLEDController = (socket) => {
  const [ledStrips, setLedStrips] = useState({
    0: { state: "OFF", brightness: 0 }, // Kitchen
    1: { state: "OFF", brightness: 0 }, // Lighting
    3: { state: "OFF", brightness: 0, mode: "OFF" }, // Bathroom (motion-activated)
    4: { state: "OFF", brightness: 0 }, // Bedroom
  });
  
  const [relays, setRelays] = useState({
    0: { state: "OFF" }, // Relay 0
  });

  useEffect(() => {
    if (!socket) {
      return;
    }

    // Listen for LED status updates
    const handleLEDStatusUpdate = (data) => {
      console.log("💡 LED Status Update:", data);

      // NEW FORMAT: Full status in one object
      if (data.type === "full" && data.data) {
        const statusData = data.data;

        // Update all strips
        if (statusData.strips) {
          const newStrips = {};
          for (const [index, stripData] of Object.entries(statusData.strips)) {
            newStrips[index] = {
              state: stripData.state,
              brightness: stripData.brightness,
              ...(stripData.mode && { mode: stripData.mode }), // Add mode if present (for Strip 3)
            };
          }
          setLedStrips((prev) => ({ ...prev, ...newStrips }));
        }

        // Update all relays
        if (statusData.relays) {
          const newRelays = {};
          for (const [index, relayData] of Object.entries(statusData.relays)) {
            newRelays[index] = {
              state: relayData.state,
            };
          }
          setRelays((prev) => ({ ...prev, ...newRelays }));
        }
      }
      // OLD FORMAT (for backward compatibility)
      else if (data.type === "strip" && typeof data.index === "number") {
        setLedStrips((prev) => ({
          ...prev,
          [data.index]: {
            ...prev[data.index],
            [data.dataType]: data.value,
          },
        }));
      } else if (data.type === "relay") {
        setRelays((prev) => ({
          ...prev,
          0: { state: data.value },
        }));
      }
    };

    socket.on("ledStatusUpdate", handleLEDStatusUpdate);

    // Cleanup
    return () => {
      socket.off("ledStatusUpdate", handleLEDStatusUpdate);
    };
  }, [socket]);

  /**
   * Send LED command to backend
   * @param {Object} command - Command object { type, index?, action, value? }
   */
  const sendLEDCommand = (command) => {
    if (!socket || !socket.connected) {
      console.warn("⚠️ Cannot send LED command - socket not connected");
      return;
    }

    socket.emit("ledCommand", command);
  };

  return {
    ledStrips,
    relays,
    sendLEDCommand,
  };
};

