/**
 * Battery detail modal — energy diagram (phase 1: static layout + mock/live data).
 */

import { BatteryEnergyDiagram } from "./BatteryEnergyDiagram";

/**
 * @param {Object} props
 * @param {number|null} props.batteryLevel
 * @param {Object} props.nodes
 * @param {Object} props.wireAmps
 * @param {Object} [props.batteryFlow]
 * @param {number|null} [props.batteryVoltage]
 * @param {Record<string, boolean>} [props.offlineByNode]
 * @param {Record<string, boolean>} [props.offlineByWire]
 * @param {boolean} [props.smartShuntOffline]
 * @param {boolean} props.disabled
 */
export function BatteryModalContent({
  batteryLevel,
  nodes,
  wireAmps,
  batteryFlow,
  batteryVoltage,
  offlineByNode,
  offlineByWire,
  smartShuntOffline,
  disabled = false,
}) {
  return (
    <div className="battery-modal">
      <BatteryEnergyDiagram
        batteryLevel={batteryLevel}
        nodes={nodes}
        wireAmps={wireAmps}
        batteryFlow={batteryFlow}
        batteryVoltage={batteryVoltage}
        offlineByNode={offlineByNode}
        offlineByWire={offlineByWire}
        smartShuntOffline={smartShuntOffline}
        disabled={disabled}
      />
    </div>
  );
}
