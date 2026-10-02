/**
 * Battery detail modal — full energy diagram, or status slide (battery + key stats).
 */

import { useMemo } from "react";
import { BatteryEnergyDiagram } from "./BatteryEnergyDiagram";
import { ModalStatSlots } from "./ModalStatSlots";
import { WaterTankModalIcon } from "./WaterTankModalIcon";
import { computeBatteryFlow, getTotalSolarPower } from "../utils/batteryWireAmps";

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
 * @param {"full"|"status"} [props.variant]
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
  variant = "full",
}) {
  const statusFlow = useMemo(() => {
    if (smartShuntOffline) {
      return {
        direction: "idle",
        amps: 0,
        watts: 0,
        netAmps: 0,
        voltage: batteryFlow?.voltage ?? batteryVoltage ?? null,
      };
    }
    return (
      batteryFlow ??
      computeBatteryFlow(wireAmps || {}, batteryVoltage ?? undefined)
    );
  }, [batteryFlow, wireAmps, smartShuntOffline, batteryVoltage]);

  const totalSolarPower = useMemo(
    () => getTotalSolarPower(wireAmps || {}),
    [wireAmps]
  );

  if (variant === "status") {
    const showValues = !disabled && !smartShuntOffline;
    const pct =
      !showValues || batteryLevel === null || batteryLevel === undefined
        ? 0
        : Math.min(100, Math.max(0, Number(batteryLevel)));
    const solarText = showValues ? `${totalSolarPower}W` : "—";
    const netAmps = Number(statusFlow?.netAmps) || 0;
    const currentText = showValues ? `${netAmps.toFixed(1)}A` : "—";

    const canOpenHistory = typeof onOpenHistory === "function";
    const handleActivate = () => {
      if (canOpenHistory) onOpenHistory();
    };

    return (
      <div className="gray-water-modal battery-modal--status">
        <div className="gray-water-modal-tank-wrap">
          <div
            className={[
              "gray-water-modal-tank",
              canOpenHistory && "gray-water-modal-tank--clickable",
            ]
              .filter(Boolean)
              .join(" ")}
            aria-label={
              showValues
                ? `Battery ${Math.round(pct)} percent${
                    canOpenHistory ? ". Open SOC history" : ""
                  }`
                : canOpenHistory
                  ? "Battery. Open SOC history"
                  : "Battery"
            }
            role={canOpenHistory ? "button" : undefined}
            tabIndex={canOpenHistory ? 0 : undefined}
            onClick={canOpenHistory ? handleActivate : undefined}
            onKeyDown={
              canOpenHistory
                ? (event) => {
                    if (event.key === "Enter" || event.key === " ") {
                      event.preventDefault();
                      handleActivate();
                    }
                  }
                : undefined
            }
          >
            <div className="gray-water-modal-overlay">
              <WaterTankModalIcon variant="battery" />
            </div>
            <div
              className="gray-water-modal-fill"
              style={{ height: `${pct}%` }}
              aria-hidden="true"
            />
          </div>
        </div>
        <ModalStatSlots
          ariaLabel="Battery summary"
          items={[
            { label: "Current", value: currentText },
            { label: "Solar Power", value: solarText },
          ]}
        />
      </div>
    );
  }

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
