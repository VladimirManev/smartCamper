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
 * @param {boolean} props.disabled
 */
export function BatteryModalContent({
  batteryLevel,
  nodes,
  wireAmps,
  batteryFlow,
  disabled = false,
}) {
  return (
    <div className="battery-modal">
      <BatteryEnergyDiagram
        batteryLevel={batteryLevel}
        nodes={nodes}
        wireAmps={wireAmps}
        batteryFlow={batteryFlow}
        disabled={disabled}
      />
    </div>
  );
}
