/**
 * Battery system diagram data from module-6 Victron BLE snapshot.
 */

import { useState, useEffect, useRef } from "react";
import { BATTERY_NODE_IDS } from "../config/batterySystemNodes";
import { mapVictronToBatterySystem } from "../utils/victronToBatterySystem";

const EMPTY_NODES = Object.fromEntries(BATTERY_NODE_IDS.map((id) => [id, null]));

const IDLE_BATTERY_FLOW = {
  direction: "idle",
  amps: 0,
  watts: 0,
  netAmps: 0,
};

/**
 * @param {Object} socket
 * @param {Object} moduleStatuses
 * @returns {{ nodes: Object, wireAmps: Object, batteryLevel: number|null, batteryFlow: Object }}
 */
export function useBatterySystem(socket, moduleStatuses) {
  const [nodes, setNodes] = useState(EMPTY_NODES);
  const [wireAmps, setWireAmps] = useState({});
  const [batteryLevel, setBatteryLevel] = useState(null);
  const [batteryFlow, setBatteryFlow] = useState(IDLE_BATTERY_FLOW);

  const moduleStatusesRef = useRef(moduleStatuses);
  moduleStatusesRef.current = moduleStatuses;

  useEffect(() => {
    if (!socket) return;

    const syncModulesFromSocket = (data) => {
      if (data?.modules) {
        moduleStatusesRef.current = data.modules;
      }
    };

    const handleUpdate = (payload) => {
      const module6Online =
        moduleStatusesRef.current["module-6"]?.status === "online";
      if (!module6Online || payload?.type !== "full" || !payload?.data) return;

      const mapped = mapVictronToBatterySystem(payload.data);
      setNodes(mapped.nodes);
      setWireAmps(mapped.wireAmps);
      setBatteryLevel(mapped.batteryLevel);
      setBatteryFlow(mapped.batteryFlow);
    };

    socket.on("moduleStatusUpdate", syncModulesFromSocket);
    socket.on("victronStatusUpdate", handleUpdate);

    return () => {
      socket.off("moduleStatusUpdate", syncModulesFromSocket);
      socket.off("victronStatusUpdate", handleUpdate);
    };
  }, [socket]);

  useEffect(() => {
    const module6Online = moduleStatuses["module-6"]?.status === "online";
    if (!module6Online) {
      setNodes(EMPTY_NODES);
      setWireAmps({});
      setBatteryLevel(null);
      setBatteryFlow(IDLE_BATTERY_FLOW);
    }
  }, [moduleStatuses]);

  return { nodes, wireAmps, batteryLevel, batteryFlow };
}
