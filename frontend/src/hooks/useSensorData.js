/**
 * useSensorData Hook
 * Manages sensor data (temperature, humidity, gray water level)
 */

import { useState, useEffect } from "react";

/**
 * Custom hook for sensor data
 * @param {Object} socket - Socket.io instance
 * @returns {Object} { temperature, humidity, grayWaterLevel }
 */
export const useSensorData = (socket) => {
  const [temperature, setTemperature] = useState(null);
  const [humidity, setHumidity] = useState(null);
  const [grayWaterLevel, setGrayWaterLevel] = useState(null);

  useEffect(() => {
    if (!socket) {
      return;
    }

    // Listen for sensor updates
    const handleSensorUpdate = (data) => {
      console.log("📊 Sensor Data Update:", data);

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
    };

    socket.on("sensorUpdate", handleSensorUpdate);

    // Cleanup
    return () => {
      socket.off("sensorUpdate", handleSensorUpdate);
    };
  }, [socket]);

  return {
    temperature,
    humidity,
    grayWaterLevel,
  };
};

