/**
 * useSensorData Hook
 * Manages sensor data (temperature, humidity, gray water level)
 * Clears data when source module goes offline
 */

import { useState, useEffect } from "react";

/**
 * Custom hook for sensor data
 * @param {Object} socket - Socket.io instance
 * @param {Function} isModuleOnline - Function to check if module is online
 * @param {Object} moduleStatuses - Object with module statuses (for dependency tracking)
 * @returns {Object} { temperature, humidity, grayWaterLevel }
 */
export const useSensorData = (socket, isModuleOnline, moduleStatuses) => {
  const [temperature, setTemperature] = useState(null);
  const [humidity, setHumidity] = useState(null);
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

      // Update temperature if present
      if (data.temperature !== undefined && data.temperature !== null) {
        setTemperature(data.temperature);
      }

      // Update humidity if present
      if (data.humidity !== undefined && data.humidity !== null) {
        setHumidity(data.humidity);
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
        setTemperature(null);
        setHumidity(null);
        setGrayWaterLevel(null);  // Clear gray water level when module-1 goes offline
        setGrayWaterTemperature(null);  // Clear gray water temperature when module-1 goes offline
      }
    } else {
      // If isModuleOnline is not available, clear data
      setTemperature(null);
      setHumidity(null);
      setGrayWaterLevel(null);
      setGrayWaterTemperature(null);
    }
  }, [isModuleOnline, moduleStatuses]);

  return {
    temperature,
    humidity,
    grayWaterLevel,
    grayWaterTemperature,
  };
};

