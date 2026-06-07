/**
 * useSensorData Hook
 * Manages sensor data from module-1 (climate, gray water), module-5 (toilet urine), module-7 (fresh water).
 */

import { useState, useEffect, useRef } from "react";

/**
 * @param {Object} socket - Socket.io instance
 * @param {Object} moduleStatuses - moduleId -> status
 */
export const useSensorData = (socket, moduleStatuses) => {
  const [indoorTemperature, setIndoorTemperature] = useState(null);
  const [indoorHumidity, setIndoorHumidity] = useState(null);
  const [outdoorTemperature, setOutdoorTemperature] = useState(null);
  const [grayWaterLevel, setGrayWaterLevel] = useState(null);
  const [grayWaterTemperature, setGrayWaterTemperature] = useState(null);
  const [toiletUrineLevel, setToiletUrineLevel] = useState(null);
  const [cleanWaterLevel, setCleanWaterLevel] = useState(null);

  const moduleStatusesRef = useRef(moduleStatuses);
  moduleStatusesRef.current = moduleStatuses;

  useEffect(() => {
    if (!socket) {
      return;
    }

    const clearSensorData = () => {
      setIndoorTemperature(null);
      setIndoorHumidity(null);
      setOutdoorTemperature(null);
      setGrayWaterLevel(null);
      setGrayWaterTemperature(null);
      setToiletUrineLevel(null);
      setCleanWaterLevel(null);
    };

    const handleDisconnect = () => clearSensorData();

    socket.on("disconnect", handleDisconnect);
    if (!socket.connected) {
      clearSensorData();
    }

    const syncModulesFromSocket = (data) => {
      if (data?.modules) {
        moduleStatusesRef.current = data.modules;
      }
    };

    const handleSensorUpdate = (data) => {
      if (!socket.connected) {
        return;
      }

      const module1Online =
        moduleStatusesRef.current["module-1"]?.status === "online";
      const module5Online =
        moduleStatusesRef.current["module-5"]?.status === "online";
      const module7Online =
        moduleStatusesRef.current["module-7"]?.status === "online";

      if (module1Online) {
        if (
          data.indoorTemperature !== undefined &&
          data.indoorTemperature !== null
        ) {
          setIndoorTemperature(data.indoorTemperature);
        }
        if (
          data.indoorHumidity !== undefined &&
          data.indoorHumidity !== null
        ) {
          setIndoorHumidity(data.indoorHumidity);
        }
        if (
          data.outdoorTemperature !== undefined &&
          data.outdoorTemperature !== null
        ) {
          setOutdoorTemperature(data.outdoorTemperature);
        }
        if (data.grayWaterLevel !== undefined && data.grayWaterLevel !== null) {
          setGrayWaterLevel(data.grayWaterLevel);
        }
        if (
          data.grayWaterTemperature !== undefined &&
          data.grayWaterTemperature !== null
        ) {
          setGrayWaterTemperature(data.grayWaterTemperature);
        }
      }

      if (module5Online) {
        if (
          data.toiletUrineLevel !== undefined &&
          data.toiletUrineLevel !== null
        ) {
          setToiletUrineLevel(data.toiletUrineLevel);
        }
      }

      if (module7Online) {
        if (
          data.cleanWaterLevel !== undefined &&
          data.cleanWaterLevel !== null
        ) {
          setCleanWaterLevel(data.cleanWaterLevel);
        }
      }
    };

    socket.on("moduleStatusUpdate", syncModulesFromSocket);
    socket.on("sensorUpdate", handleSensorUpdate);

    return () => {
      socket.off("disconnect", handleDisconnect);
      socket.off("moduleStatusUpdate", syncModulesFromSocket);
      socket.off("sensorUpdate", handleSensorUpdate);
    };
  }, [socket]);

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

  useEffect(() => {
    const module5Online = moduleStatuses["module-5"]?.status === "online";
    if (!module5Online) {
      setToiletUrineLevel(null);
    }
  }, [moduleStatuses]);

  useEffect(() => {
    const module7Online = moduleStatuses["module-7"]?.status === "online";
    if (!module7Online) {
      setCleanWaterLevel(null);
    }
  }, [moduleStatuses]);

  return {
    indoorTemperature,
    indoorHumidity,
    outdoorTemperature,
    grayWaterLevel,
    grayWaterTemperature,
    toiletUrineLevel,
    cleanWaterLevel,
  };
};
