/**
 * LEDGroupCard Component
 * Group card mirrors aggregate on-state of lighting modal strips
 * Opens a modal with all LED cards when clicked
 */

import { Card } from "./Card";
import { useEffect, useState } from "react";
import { getThemeColor } from "../utils/getThemeColor";
import { getArcProgress } from "../utils/arcProgress";

/**
 * LEDGroupCard component
 * @param {Object} props - Component props
 * @param {string} props.name - Display name (e.g., "Lighting")
 * @param {Function} props.onClick - Click handler function (opens modal)
 * @param {boolean} props.disabled - Whether the control is disabled/offline
 * @param {boolean} props.anyActive - True when any lighting-group strip is on (bathroom AUTO excluded)
 * @param {number} props.maxBrightness - Max brightness (0–255) among active strips for arc display
 */
export const LEDGroupCard = ({
  name,
  onClick,
  disabled = false,
  anyActive = false,
  maxBrightness = 0,
}) => {
  // Get theme colors for gradients only
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

  // Generate unique gradient ID based on name
  const gradientId = `gradient-${name.toLowerCase().replace(/\s+/g, "-")}-group`;

  // Lamp icon SVG (from lamp.svg - converted to use currentColor)
  const lampIcon = (
    <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
      <path d="M12.0001 7.88989L10.9301 9.74989C10.6901 10.1599 10.8901 10.4999 11.3601 10.4999H12.6301C13.1101 10.4999 13.3001 10.8399 13.0601 11.2499L12.0001 13.1099" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round" strokeLinejoin="round"/>
      <path d="M8.30011 18.0399V16.8799C6.00011 15.4899 4.11011 12.7799 4.11011 9.89993C4.11011 4.94993 8.66011 1.06993 13.8001 2.18993C16.0601 2.68993 18.0401 4.18993 19.0701 6.25993C21.1601 10.4599 18.9601 14.9199 15.7301 16.8699V18.0299C15.7301 18.3199 15.8401 18.9899 14.7701 18.9899H9.26011C8.16011 18.9999 8.30011 18.5699 8.30011 18.0399Z" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round" strokeLinejoin="round"/>
      <path d="M8.5 22C10.79 21.35 13.21 21.35 15.5 22" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round" strokeLinejoin="round"/>
    </svg>
  );

  const arcLength = Math.PI * 80 * (270 / 180);
  const progress = getArcProgress(maxBrightness, anyActive);
  const horseshoeProgress = (
    <svg className="horseshoe-progress" viewBox="0 0 200 200">
      <defs>
        <linearGradient id={gradientId} x1="0%" y1="0%" x2="0%" y2="100%">
          <stop offset="0%" stopColor={accentBlue} />
          <stop offset="100%" stopColor={accentBlueDark} />
        </linearGradient>
      </defs>
      <path
        className="horseshoe-fill"
        d="M 43.4 156.6 A 80 80 0 1 1 156.6 156.6"
        fill="none"
        stroke={`url(#${gradientId})`}
        strokeWidth="8"
        strokeLinecap="round"
        strokeDasharray={`${progress} ${arcLength}`}
        strokeDashoffset="0"
        opacity={anyActive && progress > 0 ? 1 : 0}
      />
    </svg>
  );

  const buttonState = anyActive ? "on" : "off";
  const iconState = anyActive ? "active" : "inactive";

  return (
    <Card
      name={name}
      icon={lampIcon}
      buttonState={buttonState}
      iconState={iconState}
      onClick={onClick}
      disabled={disabled}
    >
      {horseshoeProgress}
    </Card>
  );
};

