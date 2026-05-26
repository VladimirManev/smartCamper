/**
 * Battery system diagram data (modal detail).
 */

import { useState, useEffect, useRef } from "react";

const EMPTY_NODES = {
  charger230v: null,
  dcDcBooster: null,
  alternator: null,
  solarController1: null,
  solarController2: null,
  solarPanelGroup1: null,
  solarPanelGroup2: null,
  dcLoads: null,
};

/**
 * @param {Object} socket
 * @param {Object} moduleStatuses
 * @returns {{ nodes: Object }}
 */
export function useBatterySystem(socket, moduleStatuses) {
  const [nodes, setNodes] = useState(EMPTY_NODES);
  const moduleStatusesRef = useRef(moduleStatuses);
  moduleStatusesRef.current = moduleStatuses;

  useEffect(() => {
    if (!socket) return;

    const syncModulesFromSocket = (data) => {
      if (data?.modules) {
        moduleStatusesRef.current = data.modules;
      }
    };

    const handleUpdate = (data) => {
      const module1Online =
        moduleStatusesRef.current["module-1"]?.status === "online";
      if (!module1Online || !data?.nodes) return;
      setNodes({ ...EMPTY_NODES, ...data.nodes });
    };

    socket.on("moduleStatusUpdate", syncModulesFromSocket);
    socket.on("batterySystemUpdate", handleUpdate);

    return () => {
      socket.off("moduleStatusUpdate", syncModulesFromSocket);
      socket.off("batterySystemUpdate", handleUpdate);
    };
  }, [socket]);

  useEffect(() => {
    const module1Online = moduleStatuses["module-1"]?.status === "online";
    if (!module1Online) {
      setNodes(EMPTY_NODES);
    }
  }, [moduleStatuses]);

  return { nodes };
}
