/**
 * useSensorData Hook
 * Manages sensor data (indoor temperature, indoor humidity, outdoor temperature, gray water level)
 * Clears data when source module goes offline
 */

import { useState, useEffect } from "react";

/**
 * Custom hook for sensor data
 * @param {Object} socket - Socket.io instance
 * @param {Function} isModuleOnline - Function to check if module is online
 * @param {Object} moduleStatuses - Object with module statuses (for dependency tracking)
 * @returns {Object} { indoorTemperature, indoorHumidity, outdoorTemperature, grayWaterLevel, grayWaterTemperature }
 */
export const useSensorData = (socket, isModuleOnline, moduleStatuses) => {
  const [indoorTemperature, setIndoorTemperature] = useState(null);
  const [indoorHumidity, setIndoorHumidity] = useState(null);
  const [outdoorTemperature, setOutdoorTemperature] = useState(null);
  const [grayWaterLevel, setGrayWaterLevel] = useState(null);
  const [grayWaterTemperature, setGrayWaterTemperature] = useState(null);

  useEffect(() => {
    if (!socket) {
      return;
    }

    // Listen for sensor updates
    const handleSensorUpdate = (data) => {
      // Only update if module is online
      if (!isModuleOnline || !isModuleOnline("module-1")) {
        return;
      }

      // Update indoor temperature if present
      if (data.indoorTemperature !== undefined && data.indoorTemperature !== null) {
        setIndoorTemperature(data.indoorTemperature);
      }

      // Update indoor humidity if present
      if (data.indoorHumidity !== undefined && data.indoorHumidity !== null) {
        setIndoorHumidity(data.indoorHumidity);
      }

      // Update outdoor temperature if present
      if (data.outdoorTemperature !== undefined && data.outdoorTemperature !== null) {
        setOutdoorTemperature(data.outdoorTemperature);
      }

      // Update gray water level if present
      if (data.grayWaterLevel !== undefined && data.grayWaterLevel !== null) {
        setGrayWaterLevel(data.grayWaterLevel);
      }

      // Update gray water temperature if present
      if (data.grayWaterTemperature !== undefined && data.grayWaterTemperature !== null) {
        setGrayWaterTemperature(data.grayWaterTemperature);
      }
    };

    socket.on("sensorUpdate", handleSensorUpdate);

    // Cleanup
    return () => {
      socket.off("sensorUpdate", handleSensorUpdate);
    };
  }, [socket, isModuleOnline]);

  // Clear sensor data when module goes offline
  useEffect(() => {
    if (isModuleOnline) {
      const module1Online = isModuleOnline("module-1");
      if (!module1Online) {
        setIndoorTemperature(null);
        setIndoorHumidity(null);
        setOutdoorTemperature(null);
        setGrayWaterLevel(null);  // Clear gray water level when module-1 goes offline
        setGrayWaterTemperature(null);  // Clear gray water temperature when module-1 goes offline
      }
    } else {
      // If isModuleOnline is not available, clear data
      setIndoorTemperature(null);
      setIndoorHumidity(null);
      setOutdoorTemperature(null);
      setGrayWaterLevel(null);
      setGrayWaterTemperature(null);
    }
  }, [isModuleOnline, moduleStatuses]);

  return {
    indoorTemperature,
    indoorHumidity,
    outdoorTemperature,
    grayWaterLevel,
    grayWaterTemperature,
  };
};

