/**
 * Battery detail modal — energy diagram (phase 1: static layout + mock/live data).
 */

import { BatteryEnergyDiagram } from "./BatteryEnergyDiagram";

/**
 * @param {Object} props
 * @param {number|null} props.batteryLevel
 * @param {Object} props.nodes
 * @param {boolean} props.disabled
 */
export function BatteryModalContent({
  batteryLevel,
  nodes,
  disabled = false,
}) {
  return (
    <div className="battery-modal">
      <BatteryEnergyDiagram
        batteryLevel={batteryLevel}
        nodes={nodes}
        disabled={disabled}
      />
    </div>
  );
}
