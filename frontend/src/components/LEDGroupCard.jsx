/**
 * LEDGroupCard Component
 * Group card that displays as an LED card in OFF state
 * Opens a modal with all LED cards when long pressed
 */

import { useLongPress } from "../hooks/useLongPress";
import { useEffect, useState } from "react";
import { getThemeColor } from "../utils/getThemeColor";

/**
 * LEDGroupCard component
 * @param {Object} props - Component props
 * @param {string} props.name - Display name (e.g., "Lighting")
 * @param {Function} props.onClick - Click handler function (opens modal)
 * @param {boolean} props.disabled - Whether the control is disabled/offline
 */
export const LEDGroupCard = ({ name, onClick, disabled = false }) => {
  // Always show as OFF state for group card
  const isOn = false;
  const brightness = 0;
  
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
    
    // Update periodically to catch theme changes
    const interval = setInterval(updateColors, 100);
    
    return () => clearInterval(interval);
  }, []);

  // Calculate arc progress (will be 0 since brightness is 0)
  const arcLength = Math.PI * 80 * (270 / 180);
  const progress = 0;

  // Button class - always OFF state (no glow)
  const buttonClass = "neumorphic-button off";

  // Generate unique gradient ID based on name
  const gradientId = `gradient-${name.toLowerCase().replace(/\s+/g, "-")}-group`;

  // Handle click - opens modal
  const handleClick = (event) => {
    if (!disabled && onClick) {
      if (event) {
        event.preventDefault();
        event.stopPropagation();
      }
      onClick();
    }
  };

  // Handle long press - placeholder (logs to console for now)
  const handleLongPress = () => {
    if (!disabled) {
      console.log("LED Group card long pressed");
    }
  };

  // Long press handlers
  const longPressHandlers = useLongPress(handleLongPress, handleClick);

  return (
    <div className={`led-card ${disabled ? "disabled" : ""}`} {...longPressHandlers}>
      <p className="led-name">{name}</p>
      <div className={buttonClass}>
        <svg className="horseshoe-progress" viewBox="0 0 200 200">
          <defs>
            <linearGradient id={gradientId} x1="0%" y1="0%" x2="0%" y2="100%">
              <stop offset="0%" stopColor={accentBlue} />
              <stop offset="100%" stopColor={accentBlueDark} />
            </linearGradient>
          </defs>
          {/* Arc from 135° (start) to 45° (end) - not visible since progress is 0 */}
          <path
            className="horseshoe-fill"
            d="M 43.4 156.6 A 80 80 0 1 1 156.6 156.6"
            fill="none"
            stroke={`url(#${gradientId})`}
            strokeWidth="8"
            strokeLinecap="round"
            strokeDasharray={`${progress} ${arcLength}`}
            strokeDashoffset="0"
            opacity="0"
          />
        </svg>
        <span className="button-text">
          <span className="bulb-icon">
            <svg width="20" height="20" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
              {/* Light bulb icon - simple and clean */}
              <path d="M12 2C9.24 2 7 4.24 7 7c0 1.57.8 2.95 2 3.74V14c0 .55.45 1 1 1h4c.55 0 1-.45 1-1v-3.26c1.2-.79 2-2.17 2-3.74 0-2.76-2.24-5-5-5z" stroke={accentBlue} strokeWidth="2" fill="none" strokeLinecap="round" strokeLinejoin="round"/>
              <line x1="9" y1="18" x2="15" y2="18" stroke={accentBlue} strokeWidth="2" strokeLinecap="round"/>
              <line x1="10" y1="21" x2="14" y2="21" stroke={accentBlue} strokeWidth="2" strokeLinecap="round"/>
            </svg>
          </span>
        </span>
      </div>
    </div>
  );
};

