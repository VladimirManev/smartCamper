/**
 * Large vertical battery for the energy diagram (modal).
 */

import { useId } from "react";
import { getBatteryFillColor } from "../utils/batteryFillColor";

/**
 * @param {Object} props
 * @param {number|null} props.charge - 0–100
 * @param {{ direction?: 'charge' | 'discharge' | 'idle', amps?: number, watts?: number }} [props.flow]
 * @param {boolean} props.disabled
 */
export function BatteryDiagramCenter({ charge, flow, disabled = false }) {
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

  const pctY = showFlow ? 32 : 39;
  const ampsText = showFlow ? `${netAmps.toFixed(1)}A` : "";
  const wattsText = showFlow ? `${Math.round(flow?.watts ?? 0)}W` : "";

  const flowAria =
    netAmps > 0.05 ? "charging" : netAmps < -0.05 ? "discharging" : "";

  return (
    <div
      className={`battery-diagram-center ${disabled ? "battery-diagram-center--disabled" : ""}`}
      aria-label={
        hasCharge
          ? showFlow
            ? `Battery ${pct} percent, ${flowAria} ${ampsText} ${wattsText}`
            : `Battery ${pct} percent`
          : "Battery"
      }
    >
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
            <text
              className="battery-diagram-center__pct"
              x="24"
              y={pctY}
              textAnchor="middle"
              dominantBaseline="middle"
            >
              {pct}%
            </text>
            {showFlow && (
              <>
                <text
                  className="battery-diagram-center__flow-metric"
                  x="24"
                  y="44"
                  textAnchor="middle"
                  dominantBaseline="middle"
                >
                  {ampsText}
                </text>
                <text
                  className="battery-diagram-center__flow-metric"
                  x="24"
                  y="54"
                  textAnchor="middle"
                  dominantBaseline="middle"
                >
                  {wattsText}
                </text>
              </>
            )}
          </>
        )}
      </svg>
    </div>
  );
}
