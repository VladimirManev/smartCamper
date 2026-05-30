/**
 * Battery energy flow diagram — 8 nodes + central battery + measured wires.
 */

import { useCallback, useLayoutEffect, useMemo, useRef, useState } from "react";
import { BATTERY_SYSTEM_NODES, BATTERY_WIRE_LINKS } from "../config/batterySystemNodes";
import { buildWireShape } from "../utils/batteryWireGeometry";
import { deriveWireAmpsFromNodes, formatWireAmps, hasWireCurrent, computeBatteryFlow } from "../utils/batteryWireAmps";
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

function getWireElementProps(wire, amps) {
  const shouldPulse =
    hasWireCurrent(amps) && (wire.flow === "charge" || wire.flow === "discharge");

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
 * @param {boolean} props.disabled
 */
export function BatteryEnergyDiagram({
  batteryLevel,
  nodes = {},
  wireAmps = {},
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

  const batteryFlow = useMemo(
    () => computeBatteryFlow(resolvedWireAmps),
    [resolvedWireAmps]
  );

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
            const { className } = getWireElementProps(wire, amps);
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
            if (!hasWireCurrent(amps) || wire.labelX == null || wire.labelY == null) {
              return null;
            }
            return (
              <text
                key={`${wire.id}-amps`}
                className={getWireLabelClassName()}
                x={wire.labelX}
                y={wire.labelY}
                textAnchor="middle"
                dominantBaseline="middle"
              >
                {formatWireAmps(amps)}
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
                data={nodes[node.id]}
                metricKind={node.metric}
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
            disabled={disabled}
          />
        </div>
      </div>
    </div>
  );
}
