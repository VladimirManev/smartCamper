/**
 * Large vertical battery for the energy diagram (modal).
 */

import { useId } from "react";
import { getBatteryFillColor } from "../utils/batteryFillColor";

/**
 * @param {Object} props
 * @param {number|null} props.charge - 0–100
 * @param {{ direction?: 'charge' | 'discharge' | 'idle', amps?: number, watts?: number, voltage?: number|null }} [props.flow]
 * @param {number|null} [props.voltage]
 * @param {boolean} [props.dataOffline]
 * @param {boolean} props.disabled
 */
export function BatteryDiagramCenter({
  charge,
  flow,
  voltage: voltageProp,
  dataOffline = false,
  disabled = false,
}) {
  const clipId = useId().replace(/:/g, "");

  const hasCharge =
    !disabled && charge !== null && charge !== undefined && !Number.isNaN(Number(charge));

  const pct = hasCharge
    ? Math.min(100, Math.max(0, Math.round(Number(charge))))
    : 0;

  const fillColor = hasCharge ? getBatteryFillColor(pct) : null;
  const fillH = (54 * pct) / 100;

  const netAmps = Number(flow?.netAmps) || 0;
  const showFlow = hasCharge && Math.abs(netAmps) > 0.05;

  const voltage = voltageProp ?? flow?.voltage;
  const showVoltage =
    hasCharge && voltage != null && !Number.isNaN(Number(voltage));
  const voltageText = showVoltage ? `${Number(voltage).toFixed(1)}V` : "";

  const ampsText = showFlow ? `${netAmps.toFixed(1)}A` : "";
  const wattsText = showFlow ? `${Math.round(flow?.watts ?? 0)}W` : "";

  const textLines = [{ text: `${pct}%`, className: "battery-diagram-center__pct" }];
  if (showVoltage) {
    textLines.push({ text: voltageText, className: "battery-diagram-center__flow-metric" });
  }
  if (showFlow) {
    textLines.push({ text: ampsText, className: "battery-diagram-center__flow-metric" });
    textLines.push({ text: wattsText, className: "battery-diagram-center__flow-metric" });
  }

  const TEXT_LINE_GAP = 10;
  const INNER_TEXT_CENTER_Y = 39;
  const textStartY =
    INNER_TEXT_CENTER_Y - ((textLines.length - 1) * TEXT_LINE_GAP) / 2;

  const flowAria =
    netAmps > 0.05 ? "charging" : netAmps < -0.05 ? "discharging" : "";

  const ariaParts = [`Battery ${pct} percent`];
  if (showVoltage) ariaParts.push(voltageText);
  if (showFlow) ariaParts.push(flowAria, ampsText, wattsText);

  return (
    <div
      className={[
        "battery-diagram-center",
        disabled && "battery-diagram-center--disabled",
        dataOffline && "battery-diagram-center--offline",
      ]
        .filter(Boolean)
        .join(" ")}
      aria-label={
        hasCharge ? ariaParts.join(", ") : "Battery"
      }
    >
      {dataOffline && !disabled && (
        <span className="battery-diagram-center__off-badge" aria-hidden="true">
          OFF
        </span>
      )}
      <svg className="battery-diagram-center__svg" viewBox="0 0 48 72" aria-hidden="true">
        <rect
          className="battery-diagram-center__shell"
          x="6"
          y="10"
          width="36"
          height="58"
          rx="5"
        />
        <rect
          className="battery-diagram-center__cap"
          x="16"
          y="6.5"
          width="16"
          height="3.5"
          rx="1.25"
        />
        {hasCharge && fillColor && (
          <>
            <defs>
              <clipPath id={clipId}>
                <rect x="9" y="13" width="30" height="52" rx="3.5" />
              </clipPath>
            </defs>
            <rect
              className="battery-diagram-center__fill"
              clipPath={`url(#${clipId})`}
              x="9"
              y={65 - fillH}
              width="30"
              height={Math.max(fillH, 3)}
              rx="3"
              fill={fillColor}
            />
          </>
        )}
        {hasCharge && (
          <>
            {textLines.map((line, index) => (
              <text
                key={line.text}
                className={line.className}
                x="24"
                y={textStartY + index * TEXT_LINE_GAP}
                textAnchor="middle"
                dominantBaseline="middle"
              >
                {line.text}
              </text>
            ))}
          </>
        )}
      </svg>
    </div>
  );
}
