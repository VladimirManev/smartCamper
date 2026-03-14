/**
 * LEDCard Component
 * Displays and controls a single LED strip or relay
 */

import { getArcProgress } from "../utils/arcProgress";
import { useEffect, useState, memo } from "react";
import { getThemeColor } from "../utils/getThemeColor";
import { Card } from "./Card";

/**
 * LEDCard component
 * @param {Object} props - Component props
 * @param {string} props.name - Display name (e.g., "Kitchen")
 * @param {Object} props.strip - Strip state object { state, brightness, mode? }
 * @param {Function} props.onClick - Click handler function
 * @param {Function} props.onLongPress - Long press handler function (opens modal)
 * @param {string} props.type - Type: "strip" or "relay" (default: "strip")
 * @param {boolean} props.disabled - Whether the LED control is disabled/offline
 * @param {string} props.icon - Icon type: "bulb" (default), "audio", "pump", "fridge", "fan", "boiler"
 */
const LEDCardComponent = ({ name, strip, onClick, onLongPress, type = "strip", disabled = false, icon = "bulb" }) => {
  const isOn = strip?.state === "ON";
  const brightness = strip?.brightness || 0;
  const mode = strip?.mode;
  
  // Get theme colors for gradients only
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
  const showIcon = displayText === "OFF" || displayText === "ON";
  const isIconOn = isOn && displayText === "ON";

  // Generate unique gradient ID based on name and type
  const gradientId = `gradient-${name.toLowerCase().replace(/\s+/g, "-")}-${type}`;

  // Determine button state and icon state
  const buttonState = mode === "AUTO" ? "auto" : (isOn ? "on" : "off");
  const iconState = isIconOn ? "active" : "inactive";

  // Render icon based on type
  const renderIcon = () => {
    if (!showIcon) return displayText;
    
    const iconMap = {
      audio: (
        <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
          <path d="M12 3L7 8H3v8h4l5 5V3z" stroke="currentColor" strokeWidth="2" fill={isIconOn ? "currentColor" : "none"} strokeLinecap="round" strokeLinejoin="round"/>
          <path d="M17 9c1.5 1.5 1.5 4 0 5.5" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round"/>
          <path d="M20 6c2.5 2.5 2.5 6.5 0 9" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round"/>
        </svg>
      ),
      pump: (
        <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
          <path d="M12 2c0 0-7 8-7 12 0 4 3 7 7 7s7-3 7-7c0-4-7-12-7-12z" stroke="currentColor" strokeWidth="2" fill={isIconOn ? "currentColor" : "none"} strokeLinecap="round" strokeLinejoin="round"/>
        </svg>
      ),
      fridge: (
        <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
          <line x1="12" y1="2" x2="12" y2="22" stroke="currentColor" strokeWidth="2" strokeLinecap="round"/>
          <line x1="2" y1="12" x2="22" y2="12" stroke="currentColor" strokeWidth="2" strokeLinecap="round"/>
          <line x1="5.64" y1="5.64" x2="18.36" y2="18.36" stroke="currentColor" strokeWidth="2" strokeLinecap="round"/>
          <line x1="18.36" y1="5.64" x2="5.64" y2="18.36" stroke="currentColor" strokeWidth="2" strokeLinecap="round"/>
          <line x1="6" y1="12" x2="3" y2="9" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round"/>
          <line x1="6" y1="12" x2="3" y2="15" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round"/>
          <line x1="18" y1="12" x2="21" y2="9" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round"/>
          <line x1="18" y1="12" x2="21" y2="15" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round"/>
          <line x1="12" y1="6" x2="9" y2="3" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round"/>
          <line x1="12" y1="6" x2="15" y2="3" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round"/>
          <line x1="12" y1="18" x2="9" y2="21" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round"/>
          <line x1="12" y1="18" x2="15" y2="21" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round"/>
          <circle cx="12" cy="12" r="1.5" fill={isIconOn ? "currentColor" : "none"} stroke="currentColor" strokeWidth="1.5"/>
        </svg>
      ),
      fan: (
        <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
          <path d="M12 2 Q16 6 18 12 Q16 18 12 20 Q8 18 6 12 Q8 6 12 2" stroke="currentColor" strokeWidth="2.5" fill={isIconOn ? "currentColor" : "none"} strokeLinecap="round" strokeLinejoin="round"/>
          <circle cx="12" cy="12" r="2" fill={isIconOn ? "var(--color-bg-primary)" : "currentColor"} opacity={isIconOn ? 0.9 : 1}/>
        </svg>
      ),
      boiler: (
        <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
          <rect x="0" y="5" width="24" height="14" rx="2" stroke="currentColor" strokeWidth="2" fill={isIconOn ? "currentColor" : "none"} strokeLinecap="round" strokeLinejoin="round"/>
          <path d="M0 10 Q-2 8 0 6 Q-1 8 0 10" stroke="currentColor" strokeWidth="2" fill={isIconOn ? "currentColor" : "none"} strokeLinecap="round" strokeLinejoin="round"/>
          {isIconOn && (
            <path d="M0 9 Q-1 8 0 7" stroke="var(--color-bg-primary)" strokeWidth="1.5" fill="var(--color-bg-primary)" opacity="0.9"/>
          )}
        </svg>
      ),
      bulb: (
        <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
          <path d="M12 2C9.24 2 7 4.24 7 7c0 1.57.8 2.95 2 3.74V14c0 .55.45 1 1 1h4c.55 0 1-.45 1-1v-3.26c1.2-.79 2-2.17 2-3.74 0-2.76-2.24-5-5-5z" stroke="currentColor" strokeWidth="2" fill={isIconOn ? "currentColor" : "none"} strokeLinecap="round" strokeLinejoin="round"/>
          <line x1="9" y1="18" x2="15" y2="18" stroke="currentColor" strokeWidth="2" strokeLinecap="round"/>
          <line x1="10" y1="21" x2="14" y2="21" stroke="currentColor" strokeWidth="2" strokeLinecap="round"/>
        </svg>
      ),
    };
    
    return iconMap[icon] || iconMap.bulb;
  };

  // Progress arc for strips or circle for relays
  const progressArc = type === "strip" ? (
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
  );

  return (
    <Card
      name={name}
      icon={showIcon ? renderIcon() : displayText}
      buttonState={buttonState}
      iconState={iconState}
      onClick={onClick}
      onLongPress={onLongPress}
      disabled={disabled}
    >
      {progressArc}
    </Card>
  );
};

// Memoize component to prevent unnecessary re-renders
export const LEDCard = memo(LEDCardComponent);
