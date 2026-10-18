/**
 * Autoterm heater card — same visual language as Audio (LEDCard relay):
 * filled icon when on + full blue ring.
 */

import { memo, useEffect, useState } from "react";
import { Card } from "./Card";
import { getThemeColor } from "../utils/getThemeColor";

const AutotermCardComponent = ({
  name = "Heater",
  status,
  onClick,
  onLongPress,
  disabled = false,
}) => {
  const state = status?.state || "unknown";
  const locked = !!status?.controlLocked;
  const hasError = typeof status?.errorCode === "number" && status.errorCode > 0;
  const isOn =
    state === "running" ||
    state === "starting" ||
    state === "vent" ||
    state === "shutdown";

  const [accentBlue, setAccentBlue] = useState("#3b82f6");
  const [accentBlueDark, setAccentBlueDark] = useState("#2563eb");

  useEffect(() => {
    const updateColors = () => {
      setAccentBlue(getThemeColor("--color-accent-blue"));
      setAccentBlueDark(getThemeColor("--color-accent-blue-dark"));
    };
    updateColors();
    const interval = setInterval(updateColors, 2000);
    return () => clearInterval(interval);
  }, []);

  let buttonState = "off";
  if (hasError) {
    buttonState = "error";
  } else if (locked) {
    buttonState = "temp-control-off";
  } else if (isOn) {
    buttonState = "on";
  }

  const showRing = isOn || locked;
  const isIconOn = isOn || locked;
  const gradientId = "gradient-heater-autoterm-relay";

  const flameIcon = (
    <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
      <path
        d="M12 3C12 3 7 8 7 12.5C7 15.5 9.2 18 12 18C14.8 18 17 15.5 17 12.5C17 8 12 3 12 3Z"
        stroke="currentColor"
        strokeWidth="2"
        fill={isIconOn ? "currentColor" : "none"}
        strokeLinejoin="round"
      />
      <path
        d="M12 14C12 14 10.5 15.2 10.5 16.2C10.5 17 11.2 17.5 12 17.5C12.8 17.5 13.5 17 13.5 16.2C13.5 15.2 12 14 12 14Z"
        stroke={isIconOn ? "var(--color-bg-primary)" : "currentColor"}
        strokeWidth="2"
        fill={isIconOn ? "var(--color-bg-primary)" : "none"}
        strokeLinejoin="round"
        opacity={isIconOn ? 0.9 : 1}
      />
    </svg>
  );

  const ringOverlay = (
    <svg className="horseshoe-progress" viewBox="0 0 200 200">
      <defs>
        <linearGradient id={gradientId} x1="0%" y1="0%" x2="0%" y2="100%">
          <stop offset="0%" stopColor={accentBlue} />
          <stop offset="100%" stopColor={accentBlueDark} />
        </linearGradient>
      </defs>
      {showRing && (
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
  );

  return (
    <Card
      name={name}
      icon={hasError ? <span className="error-icon">!</span> : flameIcon}
      buttonState={buttonState}
      iconState={isIconOn || hasError ? "active" : "inactive"}
      onClick={onClick}
      onLongPress={onLongPress}
      disabled={disabled}
    >
      {ringOverlay}
    </Card>
  );
};

export const AutotermCard = memo(AutotermCardComponent);
