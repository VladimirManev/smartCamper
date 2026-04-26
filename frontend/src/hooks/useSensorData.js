/**
 * useSensorData Hook
 * Manages sensor data (indoor temperature, indoor humidity, outdoor temperature, gray water level)
 * Clears data when source module goes offline
 */

import { useState, useEffect, useRef } from "react";

/**
 * Custom hook for sensor data
 * @param {Object} socket - Socket.io instance
 * @param {Object} moduleStatuses - moduleId -> status (used to know if module-1 is online)
 * @returns {Object} { indoorTemperature, indoorHumidity, outdoorTemperature, grayWaterLevel, grayWaterTemperature }
 */
export const useSensorData = (socket, moduleStatuses) => {
  const [indoorTemperature, setIndoorTemperature] = useState(null);
  const [indoorHumidity, setIndoorHumidity] = useState(null);
  const [outdoorTemperature, setOutdoorTemperature] = useState(null);
  const [grayWaterLevel, setGrayWaterLevel] = useState(null);
  const [grayWaterTemperature, setGrayWaterTemperature] = useState(null);

  const moduleStatusesRef = useRef(moduleStatuses);
  moduleStatusesRef.current = moduleStatuses;

  useEffect(() => {
    if (!socket) {
      return;
    }

    // Ref is only updated on React render; moduleStatusUpdate + sensorUpdate often arrive
    // in the same tick before re-render — sync ref here so sensor handler sees module-1 online.
    const syncModulesFromSocket = (data) => {
      if (data?.modules) {
        moduleStatusesRef.current = data.modules;
      }
    };

    const handleSensorUpdate = (data) => {
      const module1Online =
        moduleStatusesRef.current["module-1"]?.status === "online";
      if (!module1Online) {
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

    socket.on("moduleStatusUpdate", syncModulesFromSocket);
    socket.on("sensorUpdate", handleSensorUpdate);

    return () => {
      socket.off("moduleStatusUpdate", syncModulesFromSocket);
      socket.off("sensorUpdate", handleSensorUpdate);
    };
  }, [socket]);

  // Clear sensor data when module-1 goes offline (ref in listener avoids stale "offline" on each render)
  useEffect(() => {
    const module1Online = moduleStatuses["module-1"]?.status === "online";
    if (!module1Online) {
      setIndoorTemperature(null);
      setIndoorHumidity(null);
      setOutdoorTemperature(null);
      setGrayWaterLevel(null);
      setGrayWaterTemperature(null);
    }
  }, [moduleStatuses]);

  return {
    indoorTemperature,
    indoorHumidity,
    outdoorTemperature,
    grayWaterLevel,
    grayWaterTemperature,
  };
};

