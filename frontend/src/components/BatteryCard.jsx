/**
 * BatteryCard — iOS-style battery indicator; % shown above the icon.
 */

import { useEffect, useId, useState } from "react";
import { getThemeColor } from "../utils/getThemeColor";
import { getBatteryFillColor } from "../utils/batteryFillColor";
import { Card } from "./Card";

const VB_W = 52;
const VB_H = 22;
const BODY = { x: 0.75, y: 1.5, w: 46.5, h: 18.5, rx: 4.25 };
const INSET = 2.35;
const INNER = {
  x: BODY.x + INSET,
  y: BODY.y + INSET,
  w: BODY.w - INSET * 2,
  h: BODY.h - INSET * 2,
  rx: 2.6,
};
/* Smaller right cap (iOS nub) */
const CAP = { x: 48.1, y: 9.35, w: 2.85, h: 5.4, rx: 1 };

/**
 * @param {Object} props
 * @param {number|null} props.charge - State of charge 0–100
 * @param {boolean} props.disabled
 * @param {Function} [props.onClick]
 * @param {Function} [props.onLongPress]
 */
export const BatteryCard = ({
  charge,
  disabled = false,
  onClick,
  onLongPress,
}) => {
  const clipId = useId().replace(/:/g, "");
  const displayCharge = disabled ? null : charge;
  const [outlineColor, setOutlineColor] = useState("rgba(255, 255, 255, 0.55)");

  useEffect(() => {
    const sync = () => {
      const primary = getThemeColor("--color-text-primary");
      setOutlineColor(
        primary.startsWith("#")
          ? `${primary}99`
          : "rgba(255, 255, 255, 0.55)"
      );
    };
    sync();
    const interval = setInterval(sync, 2000);
    return () => clearInterval(interval);
  }, []);

  const hasCharge =
    !disabled &&
    displayCharge !== null &&
    displayCharge !== undefined;

  const pct = hasCharge
    ? Math.min(100, Math.max(0, Math.round(Number(displayCharge))))
    : 0;

  const fillColor = hasCharge ? getBatteryFillColor(pct) : null;
  const fillW = (INNER.w * pct) / 100;

  const batteryContent = (
    <div
      className="battery-card-content"
      role="img"
      aria-label={hasCharge ? `Battery ${pct} percent` : undefined}
    >
      {hasCharge && (
        <span className="battery-ios-pct" aria-hidden="true">
          {pct}%
        </span>
      )}
      <svg
        className="battery-ios-icon"
        viewBox={`0 0 ${VB_W} ${VB_H}`}
        aria-hidden="true"
      >
        <rect
          className="battery-ios-body"
          x={BODY.x}
          y={BODY.y}
          width={BODY.w}
          height={BODY.h}
          rx={BODY.rx}
          stroke={outlineColor}
        />
        <rect
          className="battery-ios-cap"
          x={CAP.x}
          y={CAP.y}
          width={CAP.w}
          height={CAP.h}
          rx={CAP.rx}
          stroke={outlineColor}
        />
        {hasCharge && fillColor && (
          <>
            <defs>
              <clipPath id={clipId}>
                <rect
                  x={INNER.x}
                  y={INNER.y}
                  width={INNER.w}
                  height={INNER.h}
                  rx={INNER.rx}
                />
              </clipPath>
            </defs>
            <rect
              className="battery-ios-fill"
              clipPath={`url(#${clipId})`}
              x={INNER.x}
              y={INNER.y}
              width={Math.max(fillW, INNER.rx)}
              height={INNER.h}
              rx={INNER.rx}
              fill={fillColor}
            />
          </>
        )}
      </svg>
    </div>
  );

  return (
    <div className="battery-card-root">
      <Card
        name="Battery"
        icon={null}
        buttonState="off"
        iconState={disabled ? "gray" : "active"}
        disabled={disabled}
        cardClass="battery-card"
        onClick={onClick}
        onLongPress={onLongPress}
      >
        {batteryContent}
      </Card>
    </div>
  );
};
