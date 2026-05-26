/**
 * Large vertical battery for the energy diagram (modal).
 */

import { useId } from "react";
import { getBatteryFillColor } from "../utils/batteryFillColor";

/**
 * @param {Object} props
 * @param {number|null} props.charge - 0–100
 * @param {boolean} props.disabled
 */
export function BatteryDiagramCenter({ charge, disabled = false }) {
  const clipId = useId().replace(/:/g, "");

  const hasCharge =
    !disabled && charge !== null && charge !== undefined && !Number.isNaN(Number(charge));

  const pct = hasCharge
    ? Math.min(100, Math.max(0, Math.round(Number(charge))))
    : 0;

  const fillColor = hasCharge ? getBatteryFillColor(pct) : null;
  const fillH = (54 * pct) / 100;

  return (
    <div
      className={`battery-diagram-center ${disabled ? "battery-diagram-center--disabled" : ""}`}
      aria-label={hasCharge ? `Battery ${pct} percent` : "Battery"}
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
          x="18"
          y="4"
          width="12"
          height="6"
          rx="2"
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
          <text
            className="battery-diagram-center__pct"
            x="24"
            y="42"
            textAnchor="middle"
            dominantBaseline="middle"
          >
            {pct}%
          </text>
        )}
      </svg>
    </div>
  );
}
