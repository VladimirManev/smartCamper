/**
 * Battery energy flow diagram — 8 nodes + central battery + measured wires.
 */

import { useCallback, useLayoutEffect, useMemo, useRef, useState } from "react";
import {
  BATTERY_SYSTEM_NODES,
  BATTERY_WIRE_LINKS,
  BATTERY_NODES_WITHOUT_OFF_BADGE,
  BATTERY_WIRE_POWER_LABELS,
} from "../config/batterySystemNodes";
import { buildWireShape } from "../utils/batteryWireGeometry";
import {
  deriveWireAmpsFromNodes,
  formatWireLabel,
  formatWirePower,
  getTotalSolarPower,
  hasWireLabelValue,
  computeBatteryFlow,
} from "../utils/batteryWireAmps";
import { BatteryNodeCard } from "./BatteryNodeCard";
import { BatteryDiagramCenter } from "./BatteryDiagramCenter";

function getWireClassName(flow) {
  if (flow === "charge") {
    return "battery-energy-diagram__wire battery-energy-diagram__wire--pulse-charge";
  }
  if (flow === "discharge") {
    return "battery-energy-diagram__wire battery-energy-diagram__wire--pulse-discharge";
  }
  return "battery-energy-diagram__wire";
}

function getWireLabelClassName() {
  return "battery-energy-diagram__wire-label";
}

function getWireElementProps(wire, value, isOffline) {
  const shouldPulse =
    !isOffline &&
    hasWireLabelValue(wire.id, value) &&
    (wire.flow === "charge" || wire.flow === "discharge");

  if (!shouldPulse) {
    return { className: "battery-energy-diagram__wire" };
  }

  return { className: getWireClassName(wire.flow) };
}

/**
 * @param {Object} props
 * @param {number|null} props.batteryLevel
 * @param {Object} props.nodes - id -> metrics object
 * @param {Object} props.wireAmps - wire id -> amps
 * @param {Object} [props.batteryFlow] - net flow from SmartShunt (preferred)
 * @param {number|null} [props.batteryVoltage]
 * @param {Record<string, boolean>} [props.offlineByNode]
 * @param {Record<string, boolean>} [props.offlineByWire]
 * @param {boolean} [props.smartShuntOffline]
 * @param {boolean} props.disabled
 */
export function BatteryEnergyDiagram({
  batteryLevel,
  nodes = {},
  wireAmps = {},
  batteryFlow: batteryFlowProp,
  batteryVoltage = null,
  offlineByNode = {},
  offlineByWire = {},
  smartShuntOffline = false,
  disabled = false,
}) {
  const containerRef = useRef(null);
  const batterySlotRef = useRef(null);
  const slotRefs = useRef({});
  const [wires, setWires] = useState([]);
  const [svgSize, setSvgSize] = useState({ w: 0, h: 0 });

  const setSlotRef = useCallback((nodeId) => {
    return (el) => {
      if (el) slotRefs.current[nodeId] = el;
      else delete slotRefs.current[nodeId];
    };
  }, []);

  const resolvedWireAmps = useMemo(() => {
    if (Object.keys(wireAmps).length > 0) return wireAmps;
    return deriveWireAmpsFromNodes(nodes);
  }, [wireAmps, nodes]);

  const totalSolarPower = useMemo(
    () => getTotalSolarPower(resolvedWireAmps),
    [resolvedWireAmps]
  );

  const batteryFlow = useMemo(() => {
    if (smartShuntOffline) {
      return {
        direction: "idle",
        amps: 0,
        watts: 0,
        netAmps: 0,
        voltage: batteryFlowProp?.voltage ?? null,
      };
    }
    return batteryFlowProp ?? computeBatteryFlow(resolvedWireAmps);
  }, [batteryFlowProp, resolvedWireAmps, smartShuntOffline]);

  useLayoutEffect(() => {
    const container = containerRef.current;
    if (!container) return;

    const measure = () => {
      const containerRect = container.getBoundingClientRect();
      if (containerRect.width <= 0 || containerRect.height <= 0) return;

      const shapes = BATTERY_WIRE_LINKS.map((link) =>
        buildWireShape(link, slotRefs.current, batterySlotRef.current, containerRect)
      ).filter(Boolean);

      setWires(shapes);
      setSvgSize({ w: containerRect.width, h: containerRect.height });
    };

    measure();

    const ro = new ResizeObserver(measure);
    ro.observe(container);
    window.addEventListener("resize", measure);

    return () => {
      ro.disconnect();
      window.removeEventListener("resize", measure);
    };
  }, [batteryLevel, nodes, disabled]);

  return (
    <div
      ref={containerRef}
      className={`battery-energy-diagram ${disabled ? "battery-energy-diagram--disabled" : ""}`}
    >
      {svgSize.w > 0 && svgSize.h > 0 && (
        <svg
          className="battery-energy-diagram__wires"
          viewBox={`0 0 ${svgSize.w} ${svgSize.h}`}
          preserveAspectRatio="none"
          aria-hidden="true"
        >
          {wires.map((wire) => {
            const amps = resolvedWireAmps[wire.id];
            const wireOffline = offlineByWire[wire.id] === true;
            const { className } = getWireElementProps(wire, amps, wireOffline);
            return wire.type === "path" ? (
              <path key={wire.id} className={className} d={wire.d} />
            ) : (
              <line
                key={wire.id}
                className={className}
                x1={wire.x1}
                y1={wire.y1}
                x2={wire.x2}
                y2={wire.y2}
              />
            );
          })}
          {wires.map((wire) => {
            const amps = resolvedWireAmps[wire.id];
            const wireOffline = offlineByWire[wire.id] === true;
            if (!hasWireLabelValue(wire.id, amps) || wire.labelX == null || wire.labelY == null) {
              return null;
            }
            const isSolarWire = BATTERY_WIRE_POWER_LABELS.has(wire.id);
            const showSolarTotal = isSolarWire && totalSolarPower > 0;

            return (
              <text
                key={`${wire.id}-amps`}
                className={`${getWireLabelClassName()}${
                  wireOffline ? " battery-energy-diagram__wire-label--offline" : ""
                }`}
                x={wire.labelX}
                y={wire.labelY}
                textAnchor="middle"
                dominantBaseline="middle"
              >
                {isSolarWire ? (
                  <>
                    <tspan x={wire.labelX} dy={showSolarTotal ? "-0.45em" : "0"}>
                      {formatWirePower(amps)}
                    </tspan>
                    {showSolarTotal && (
                      <tspan x={wire.labelX} dy="1.05em">
                        ({totalSolarPower}W)
                      </tspan>
                    )}
                  </>
                ) : (
                  formatWireLabel(wire.id, amps)
                )}
              </text>
            );
          })}
        </svg>
      )}

      <div className="battery-energy-diagram__grid">
        {BATTERY_SYSTEM_NODES.map((node) => (
          <div
            key={node.id}
            ref={setSlotRef(node.id)}
            className={`battery-energy-diagram__slot battery-energy-diagram__slot--${node.slot}`}
          >
            <div className="card-wrapper battery-energy-diagram__card-wrap">
              <BatteryNodeCard
                label={node.label}
                icon={node.icon}
                image={node.image}
                deviceLabel={node.deviceLabel}
                iconKind={node.iconKind}
                largeIcon={node.largeIcon}
                iconStyle={node.iconStyle}
                dataOffline={
                  offlineByNode[node.id] === true &&
                  !BATTERY_NODES_WITHOUT_OFF_BADGE.has(node.id)
                }
                disabled={disabled}
              />
            </div>
          </div>
        ))}
        <div
          ref={batterySlotRef}
          className="battery-energy-diagram__slot battery-energy-diagram__slot--battery"
        >
          <BatteryDiagramCenter
            charge={batteryLevel}
            flow={batteryFlow}
            voltage={batteryVoltage ?? batteryFlow?.voltage}
            dataOffline={smartShuntOffline}
            disabled={disabled}
          />
        </div>
      </div>
    </div>
  );
}
