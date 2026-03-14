/**
 * FloorHeatingCard Component
 * Displays and controls a single floor heating circle
 */

import { useState, useEffect, memo } from "react";
import { getThemeColor } from "../utils/getThemeColor";
import { Card } from "./Card";

/**
 * FloorHeatingCard component
 * @param {Object} props - Component props
 * @param {string} props.name - Display name (e.g., "Central 1")
 * @param {Object} props.circle - Circle state object { mode, relay, temperature, error? }
 * @param {Function} props.onClick - Click handler function
 * @param {Function} props.onLongPress - Long press handler function (opens modal)
 * @param {boolean} props.disabled - Whether the control is disabled/offline
 */
const FloorHeatingCardComponent = ({ name, circle, onClick, onLongPress, disabled = false }) => {
  // Get theme colors
  const [accentBlue, setAccentBlue] = useState("#3b82f6");
  const [accentBlueDark, setAccentBlueDark] = useState("#2563eb");
  
  useEffect(() => {
    const updateColors = () => {
      setAccentBlue(getThemeColor("--color-accent-blue"));
      setAccentBlueDark(getThemeColor("--color-accent-blue-dark"));
    };
    
    // Update on mount
    updateColors();
    
    // Update periodically to catch theme changes (reduced frequency for better performance)
    const interval = setInterval(updateColors, 2000); // 2 seconds instead of 100ms
    
    return () => clearInterval(interval);
  }, []);
  const mode = circle?.mode || "OFF";
  const relay = circle?.relay || "OFF";
  const temperature = circle?.temperature;
  const hasError = circle?.error || false;
  
  const isOff = mode === "OFF";
  const isTempControl = mode === "TEMP_CONTROL";
  const isRelayOn = relay === "ON";

  // Determine button state
  let buttonState = "off";
  if (hasError) {
    buttonState = "error";
  } else if (isTempControl && isRelayOn) {
    buttonState = "on";
  } else if (isTempControl && !isRelayOn) {
    buttonState = "temp-control-off";
  }

  // Generate unique gradient ID based on name
  const gradientId = `gradient-${name.toLowerCase().replace(/\s+/g, "-")}-heating`;

  // Progress circle for TEMP_CONTROL mode
  const progressCircle = (
    <svg className="horseshoe-progress" viewBox="0 0 200 200">
      <defs>
        <linearGradient id={gradientId} x1="0%" y1="0%" x2="0%" y2="100%">
          <stop offset="0%" stopColor={accentBlue} />
          <stop offset="100%" stopColor={accentBlueDark} />
        </linearGradient>
      </defs>
      {isTempControl && (
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

  // Content inside button
  const buttonContent = hasError ? (
    <span className="error-icon">!</span>
  ) : isOff ? (
    <svg className="heating-symbol" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
      <path d="M 8 18 Q 6 15, 8 12 T 8 6" stroke="currentColor" strokeWidth="2.5" strokeLinecap="round" fill="none"/>
      <path d="M 12 18 Q 10 15, 12 12 T 12 6" stroke="currentColor" strokeWidth="2.5" strokeLinecap="round" fill="none"/>
      <path d="M 16 18 Q 14 15, 16 12 T 16 6" stroke="currentColor" strokeWidth="2.5" strokeLinecap="round" fill="none"/>
    </svg>
  ) : (
    <span className="temperature-display">
      {temperature !== null && temperature !== undefined ? Math.round(temperature) : "--"}°
    </span>
  );

  // Icon state
  const iconState = isOff ? "inactive" : "active";

  return (
    <Card
      name={name}
      icon={buttonContent}
      buttonState={buttonState}
      iconState={iconState}
      onClick={onClick}
      onLongPress={onLongPress}
      disabled={disabled}
      cardClass="floor-heating-card"
    >
      {progressCircle}
    </Card>
  );
};

// Memoize component to prevent unnecessary re-renders
export const FloorHeatingCard = memo(FloorHeatingCardComponent);
