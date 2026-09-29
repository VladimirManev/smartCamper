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
 * @param {number|null} [props.batteryTemperature]
 * @param {Record<string, boolean>} [props.offlineByNode]
 * @param {Record<string, boolean>} [props.offlineByWire]
 * @param {Record<string, string|null>} [props.phaseByNode]
 * @param {boolean} [props.smartShuntOffline]
 * @param {boolean} props.disabled
 * @param {Function} [props.onOpenHistory]
 */
export function BatteryModalContent({
  batteryLevel,
  nodes,
  wireAmps,
  batteryFlow,
  batteryVoltage,
  batteryTemperature,
  offlineByNode,
  offlineByWire,
  phaseByNode,
  smartShuntOffline,
  disabled = false,
  onOpenHistory,
}) {
  return (
    <div className="battery-modal">
      <BatteryEnergyDiagram
        batteryLevel={batteryLevel}
        nodes={nodes}
        wireAmps={wireAmps}
        batteryFlow={batteryFlow}
        batteryVoltage={batteryVoltage}
        batteryTemperature={batteryTemperature}
        offlineByNode={offlineByNode}
        offlineByWire={offlineByWire}
        phaseByNode={phaseByNode}
        smartShuntOffline={smartShuntOffline}
        disabled={disabled}
        onOpenHistory={onOpenHistory}
      />
    </div>
  );
}
