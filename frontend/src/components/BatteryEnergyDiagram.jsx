/**
 * Battery energy flow diagram — 8 nodes, central battery, static wires (phase 1).
 */

import { BATTERY_SYSTEM_NODES, BATTERY_WIRE_PATHS } from "../config/batterySystemNodes";
import { BatteryNodeCard } from "./BatteryNodeCard";
import { BatteryDiagramCenter } from "./BatteryDiagramCenter";

/**
 * @param {Object} props
 * @param {number|null} props.batteryLevel
 * @param {Object} props.nodes - id -> metrics object
 * @param {boolean} props.disabled
 */
export function BatteryEnergyDiagram({
  batteryLevel,
  nodes = {},
  disabled = false,
}) {
  return (
    <div
      className={`battery-energy-diagram ${disabled ? "battery-energy-diagram--disabled" : ""}`}
    >
      <svg
        className="battery-energy-diagram__wires"
        viewBox="0 0 100 100"
        preserveAspectRatio="none"
        aria-hidden="true"
      >
        <defs>
          <marker
            id="battery-wire-arrow"
            markerWidth="4"
            markerHeight="4"
            refX="3.2"
            refY="2"
            orient="auto"
          >
            <path d="M0,0 L4,2 L0,4 Z" className="battery-energy-diagram__arrowhead" />
          </marker>
        </defs>
        {BATTERY_WIRE_PATHS.map((wire) => (
          <path
            key={wire.id}
            className="battery-energy-diagram__wire"
            d={wire.d}
            markerEnd="url(#battery-wire-arrow)"
          />
        ))}
      </svg>

      <div className="battery-energy-diagram__grid">
        {BATTERY_SYSTEM_NODES.map((node) => (
          <div
            key={node.id}
            className={`battery-energy-diagram__slot battery-energy-diagram__slot--${node.slot}`}
          >
            <BatteryNodeCard
              label={node.label}
              icon={node.icon}
              data={nodes[node.id]}
              metricKind={node.metric}
              disabled={disabled}
            />
          </div>
        ))}
        <div className="battery-energy-diagram__slot battery-energy-diagram__slot--battery">
          <BatteryDiagramCenter charge={batteryLevel} disabled={disabled} />
        </div>
      </div>
    </div>
  );
}
