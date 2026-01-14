/**
 * LEDCard Component
 * Displays and controls a single LED strip or relay
 */

import { getArcProgress } from "../utils/arcProgress";
import { useLongPress } from "../hooks/useLongPress";
import { useEffect, useState } from "react";
import { getThemeColor } from "../utils/getThemeColor";

/**
 * LEDCard component
 * @param {Object} props - Component props
 * @param {string} props.name - Display name (e.g., "Kitchen")
 * @param {Object} props.strip - Strip state object { state, brightness, mode? }
 * @param {Function} props.onClick - Click handler function
 * @param {Function} props.onLongPress - Long press handler function (opens modal)
 * @param {string} props.type - Type: "strip" or "relay" (default: "strip")
 * @param {boolean} props.disabled - Whether the LED control is disabled/offline
 */
export const LEDCard = ({ name, strip, onClick, onLongPress, type = "strip", disabled = false }) => {
  const isOn = strip?.state === "ON";
  const brightness = strip?.brightness || 0;
  const mode = strip?.mode;
  
  // Get theme colors for gradients
  const [accentBlue, setAccentBlue] = useState("#3b82f6");
  const [accentBlueDark, setAccentBlueDark] = useState("#2563eb");
  
  useEffect(() => {
    setAccentBlue(getThemeColor("--color-accent-blue"));
    setAccentBlueDark(getThemeColor("--color-accent-blue-dark"));
  }, []);

  // Calculate arc progress for strips
  const arcLength = Math.PI * 80 * (270 / 180);
  const progress = type === "strip" ? getArcProgress(brightness, isOn) : 0;

  // Determine button class based on state
  let buttonClass = "neumorphic-button";
  if (type === "relay") {
    buttonClass += isOn ? " on" : " off";
  } else {
    // For strips with mode (Bathroom)
    if (mode === "AUTO") {
      buttonClass += " auto";
    } else {
      buttonClass += isOn ? " on" : " off";
    }
  }

  // Determine display text or icon
  const displayText = mode === "AUTO" ? "AUTO" : strip?.state || "OFF";
  const showBulbIcon = displayText === "OFF" || displayText === "ON";
  const isBulbOn = isOn && displayText === "ON";

  // Generate unique gradient ID based on name and type
  // Use a simple hash to ensure uniqueness
  const gradientId = `gradient-${name.toLowerCase().replace(/\s+/g, "-")}-${type}`;

  // Handle click - don't do anything if disabled
  const handleClick = () => {
    if (!disabled && onClick) {
      onClick();
    }
  };

  // Handle long press
  const handleLongPress = () => {
    if (!disabled && onLongPress) {
      onLongPress();
    }
  };

  // Long press handlers
  const longPressHandlers = useLongPress(handleLongPress, handleClick);

  return (
    <div className={`led-card ${disabled ? "disabled" : ""}`} {...longPressHandlers}>
      <p className="led-name">{name}</p>
      <div className={buttonClass}>
        {type === "strip" ? (
          <svg className="horseshoe-progress" viewBox="0 0 200 200">
            <defs>
              <linearGradient id={gradientId} x1="0%" y1="0%" x2="0%" y2="100%">
                <stop offset="0%" stopColor={accentBlue} />
                <stop offset="100%" stopColor={accentBlueDark} />
              </linearGradient>
            </defs>
            {/* Arc from 135° (start) to 45° (end) - fills according to brightness */}
            <path
              className="horseshoe-fill"
              d="M 43.4 156.6 A 80 80 0 1 1 156.6 156.6"
              fill="none"
              stroke={`url(#${gradientId})`}
              strokeWidth="8"
              strokeLinecap="round"
              strokeDasharray={`${progress} ${arcLength}`}
              strokeDashoffset="0"
              opacity={isOn && progress > 0 ? 1 : 0}
            />
          </svg>
        ) : (
          <svg className="horseshoe-progress" viewBox="0 0 200 200">
            <defs>
              <linearGradient id={gradientId} x1="0%" y1="0%" x2="0%" y2="100%">
                <stop offset="0%" stopColor={accentBlue} />
                <stop offset="100%" stopColor={accentBlueDark} />
              </linearGradient>
            </defs>
            {/* Closed circle - if ON it exists, if OFF it doesn't */}
            {isOn && (
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
        )}
        <span className="button-text">
          {showBulbIcon ? (
            <span className={`bulb-icon ${isBulbOn ? "bulb-on" : ""}`}>
              <svg width="20" height="20" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
                {/* Light bulb icon - simple and clean */}
                <path d="M12 2C9.24 2 7 4.24 7 7c0 1.57.8 2.95 2 3.74V14c0 .55.45 1 1 1h4c.55 0 1-.45 1-1v-3.26c1.2-.79 2-2.17 2-3.74 0-2.76-2.24-5-5-5z" stroke="currentColor" strokeWidth="2" fill={isBulbOn ? "currentColor" : "none"} strokeLinecap="round" strokeLinejoin="round"/>
                <line x1="9" y1="18" x2="15" y2="18" stroke="currentColor" strokeWidth="2" strokeLinecap="round"/>
                <line x1="10" y1="21" x2="14" y2="21" stroke="currentColor" strokeWidth="2" strokeLinecap="round"/>
              </svg>
            </span>
          ) : (
            displayText
          )}
        </span>
      </div>
    </div>
  );
};

