/**
 * Boiler thermostat status from module-5.
 * Relay index 4 in appliance status is "enabled", not element current.
 */

import { useEffect, useState } from "react";

const DEFAULT_BOILER = {
  enabled: false,
  relay: false,
  setpoint: 60,
  top: null,
  bottom: null,
  full: null,
  error: null,
};

function temperatureOrNull(value) {
  if (value === null || value === undefined) {
    return null;
  }
  const number = Number(value);
  if (!Number.isInteger(number)) {
    return null;
  }
  return number;
}

function sanitizeBoiler(data) {
  const setpoint = Number(data.setpoint);
  return {
    enabled: data.enabled === true,
    relay: data.relay === true,
    setpoint:
      Number.isInteger(setpoint) && setpoint >= 5 && setpoint <= 70
        ? setpoint
        : DEFAULT_BOILER.setpoint,
    top: temperatureOrNull(data.top),
    bottom: temperatureOrNull(data.bottom),
    full: data.full === true ? true : data.full === false ? false : null,
    error:
      data.error === "empty" ||
      data.error === "sensor" ||
      data.error === "overtemp"
        ? data.error
        : null,
  };
}

export function useBoiler(socket) {
  const [boiler, setBoiler] = useState(DEFAULT_BOILER);

  useEffect(() => {
    if (!socket) {
      return;
    }

    const handleBoilerStatus = (data) => {
      if (!data || typeof data !== "object") {
        return;
      }
      setBoiler(sanitizeBoiler(data));
    };

    socket.on("boilerStatusUpdate", handleBoilerStatus);
    return () => {
      socket.off("boilerStatusUpdate", handleBoilerStatus);
    };
  }, [socket]);

  return boiler;
}
