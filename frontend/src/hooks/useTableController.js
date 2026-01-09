/**
 * useTableController Hook
 * Manages table lift state and commands
 * Handles table commands to backend
 */

import { useState, useEffect } from "react";

/**
 * Custom hook for table controller
 * @param {Object} socket - Socket.io instance
 * @returns {Object} { tableState, sendTableCommand }
 */
export const useTableController = (socket) => {
  // Table state: "up", "down", or "stopped"
  const [tableState, setTableState] = useState({
    direction: "stopped",
  });

  useEffect(() => {
    if (!socket) {
      return;
    }

    // Listen for table status updates
    const handleTableStatusUpdate = (data) => {
      console.log("🪑 Table Status Update:", data);

      if (data.type === "table" && data.direction !== undefined) {
        setTableState({
          direction: data.direction,
        });
      }
    };

    socket.on("tableStatusUpdate", handleTableStatusUpdate);

    // Cleanup
    return () => {
      socket.off("tableStatusUpdate", handleTableStatusUpdate);
    };
  }, [socket]);

  /**
   * Send table command to backend
   * @param {Object} command - Command object { type, action, duration? }
   */
  const sendTableCommand = (command) => {
    if (!socket || !socket.connected) {
      console.warn("⚠️ Cannot send table command - socket not connected");
      return;
    }

    socket.emit("tableCommand", command);
  };

  return {
    tableState,
    sendTableCommand,
  };
};

