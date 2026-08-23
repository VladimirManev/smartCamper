/**
 * PerimeterCard — main-menu entry for perimeter watch (zone 2).
 * Matches relay cards (Inverter / Pump): SVG icon + blue ring when armed.
 */

import { useEffect, useId, useState } from "react";
import { Card } from "./Card";
import { getThemeColor } from "../utils/getThemeColor";

/**
 * @param {Object} props
 * @param {string} props.name
 * @param {Function} props.onClick
 * @param {Function} [props.onLongPress]
 * @param {boolean} [props.disabled]
 * @param {boolean} [props.armed]
 */
export function PerimeterCard({
  name,
  onClick,
  onLongPress,
  disabled = false,
  armed = false,
}) {
  const [accentBlue, setAccentBlue] = useState("#3b82f6");
  const [accentBlueDark, setAccentBlueDark] = useState("#2563eb");
  const reactId = useId().replace(/:/g, "");
  const gradientId = `perimeter-card-grad-${reactId}`;

  useEffect(() => {
    const updateColors = () => {
      setAccentBlue(getThemeColor("--color-accent-blue"));
      setAccentBlueDark(getThemeColor("--color-accent-blue-dark"));
    };
    updateColors();
    const interval = setInterval(updateColors, 2000);
    return () => clearInterval(interval);
  }, []);

  const icon = (
    <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
      <path
        d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8S1 12 1 12z"
        stroke="currentColor"
        strokeWidth="2"
        fill={armed ? "currentColor" : "none"}
        strokeLinecap="round"
        strokeLinejoin="round"
      />
      <circle
        cx="12"
        cy="12"
        r="3"
        stroke="currentColor"
        strokeWidth="2"
        fill="none"
      />
    </svg>
  );

  return (
    <Card
      name={name}
      icon={icon}
      buttonState={armed ? "on" : "off"}
      iconState={disabled ? "gray" : armed ? "active" : "inactive"}
      onClick={onClick}
      onLongPress={onLongPress}
      disabled={disabled}
    >
      <svg className="horseshoe-progress" viewBox="0 0 200 200" aria-hidden>
        <defs>
          <linearGradient id={gradientId} x1="0%" y1="0%" x2="0%" y2="100%">
            <stop offset="0%" stopColor={accentBlue} />
            <stop offset="100%" stopColor={accentBlueDark} />
          </linearGradient>
        </defs>
        {armed && (
          <circle
            className="horseshoe-fill"
            cx="100"
            cy="100"
            r="80"
            fill="none"
            stroke={`url(#${gradientId})`}
            strokeWidth="8"
            strokeLinecap="round"
          />
        )}
      </svg>
    </Card>
  );
}
