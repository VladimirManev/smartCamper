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
  voltage: null,
};

const EMPTY_OFFLINE_BY_NODE = Object.fromEntries(
  BATTERY_NODE_IDS.map((id) => [id, true])
);

/**
 * @param {Object} socket
 * @param {Object} moduleStatuses
 * @returns {{
 *   nodes: Object,
 *   wireAmps: Object,
 *   batteryLevel: number|null,
 *   batteryFlow: Object,
 *   batteryVoltage: number|null,
 *   offlineByNode: Record<string, boolean>,
 *   offlineByWire: Record<string, boolean>,
 *   smartShuntOffline: boolean,
 * }}
 */
export function useBatterySystem(socket, moduleStatuses) {
  const [nodes, setNodes] = useState(EMPTY_NODES);
  const [wireAmps, setWireAmps] = useState({});
  const [batteryLevel, setBatteryLevel] = useState(null);
  const [batteryFlow, setBatteryFlow] = useState(IDLE_BATTERY_FLOW);
  const [batteryVoltage, setBatteryVoltage] = useState(null);
  const [offlineByNode, setOfflineByNode] = useState(EMPTY_OFFLINE_BY_NODE);
  const [offlineByWire, setOfflineByWire] = useState({});
  const [smartShuntOffline, setSmartShuntOffline] = useState(true);

  const moduleStatusesRef = useRef(moduleStatuses);
  moduleStatusesRef.current = moduleStatuses;

  useEffect(() => {
    if (!socket) return;

    const clearBatteryState = () => {
      setNodes(EMPTY_NODES);
      setWireAmps({});
      setBatteryLevel(null);
      setBatteryFlow(IDLE_BATTERY_FLOW);
      setBatteryVoltage(null);
      setOfflineByNode(EMPTY_OFFLINE_BY_NODE);
      setOfflineByWire({});
      setSmartShuntOffline(true);
    };

    const handleDisconnect = () => clearBatteryState();

    socket.on("disconnect", handleDisconnect);
    if (!socket.connected) {
      clearBatteryState();
    }

    const syncModulesFromSocket = (data) => {
      if (data?.modules) {
        moduleStatusesRef.current = data.modules;
      }
    };

    const handleUpdate = (payload) => {
      const module6Status = moduleStatusesRef.current["module-6"]?.status;
      if (module6Status === "offline" || payload?.type !== "full" || !payload?.data) {
        return;
      }

      const mapped = mapVictronToBatterySystem(payload.data);
      setNodes(mapped.nodes);
      setWireAmps(mapped.wireAmps);
      setBatteryLevel(mapped.batteryLevel);
      setBatteryFlow(mapped.batteryFlow);
      setBatteryVoltage(mapped.batteryVoltage);
      setOfflineByNode(mapped.offlineByNode);
      setOfflineByWire(mapped.offlineByWire);
      setSmartShuntOffline(mapped.smartShuntOffline);
    };

    socket.on("moduleStatusUpdate", syncModulesFromSocket);
    socket.on("victronStatusUpdate", handleUpdate);

    return () => {
      socket.off("disconnect", handleDisconnect);
      socket.off("moduleStatusUpdate", syncModulesFromSocket);
      socket.off("victronStatusUpdate", handleUpdate);
    };
  }, [socket]);

  useEffect(() => {
    const module6Status = moduleStatuses["module-6"]?.status;
    if (module6Status === undefined) {
      return;
    }
    if (module6Status !== "online") {
      setNodes(EMPTY_NODES);
      setWireAmps({});
      setBatteryLevel(null);
      setBatteryFlow(IDLE_BATTERY_FLOW);
      setBatteryVoltage(null);
      setOfflineByNode(EMPTY_OFFLINE_BY_NODE);
      setOfflineByWire({});
      setSmartShuntOffline(true);
    }
  }, [moduleStatuses]);

  return {
    nodes,
    wireAmps,
    batteryLevel,
    batteryFlow,
    batteryVoltage,
    offlineByNode,
    offlineByWire,
    smartShuntOffline,
  };
}
