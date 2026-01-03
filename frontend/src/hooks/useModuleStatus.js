/**
 * useModuleStatus Hook
 * Tracks module status from backend heartbeat system
 * Replaces old timeout-based tracking
 */

import { useState, useEffect } from "react";

/**
 * Custom hook for tracking module statuses
 * @param {Object} socket - Socket.io instance
 * @returns {Object} { moduleStatuses, isModuleOnline }
 */
export const useModuleStatus = (socket) => {
  const [moduleStatuses, setModuleStatuses] = useState({});

  useEffect(() => {
    if (!socket) {
      console.log("⚠️ useModuleStatus: No socket available");
      return;
    }

    console.log("👂 useModuleStatus: Setting up listener for moduleStatusUpdate");

    // Listen for module status updates from backend
    const handleModuleStatusUpdate = (data) => {
      console.log("📊 Module Status Update received:", data);
      
      if (data.modules) {
        console.log("📊 Updating module statuses:", Object.keys(data.modules));
        setModuleStatuses(data.modules);
      } else {
        console.warn("⚠️ Module Status Update missing 'modules' property:", data);
      }
    };

    socket.on("moduleStatusUpdate", handleModuleStatusUpdate);

    // Cleanup
    return () => {
      console.log("🧹 useModuleStatus: Cleaning up listener");
      socket.off("moduleStatusUpdate", handleModuleStatusUpdate);
    };
  }, [socket]);

  /**
   * Check if a specific module is online
   * @param {string} moduleId - Module identifier (e.g., "module-1")
   * @returns {boolean} True if module is online
   */
  const isModuleOnline = (moduleId) => {
    const module = moduleStatuses[moduleId];
    return module?.status === "online";
  };

  /**
   * Get status for a specific module
   * @param {string} moduleId - Module identifier
   * @returns {Object|null} Module status object or null
   */
  const getModuleStatus = (moduleId) => {
    return moduleStatuses[moduleId] || null;
  };

  return {
    moduleStatuses,
    isModuleOnline,
    getModuleStatus,
  };
};

