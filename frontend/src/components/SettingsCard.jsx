/**
 * SettingsCard Component
 * Settings card with gear icon
 * Opens a modal when clicked, logs to console on long press
 */

import { useState, useEffect } from "react";
import { useLongPress } from "../hooks/useLongPress";
import { getThemeColor } from "../utils/getThemeColor";

/**
 * SettingsCard component
 * @param {Object} props - Component props
 * @param {string} props.name - Display name (e.g., "Settings")
 * @param {Function} props.onClick - Click handler function (opens modal)
 * @param {Function} props.onLongPress - Long press handler function (logs to console)
 * @param {boolean} props.disabled - Whether the control is disabled
 */
export const SettingsCard = ({ name, onClick, onLongPress, disabled = false }) => {
  // Get theme color for icon
  const [accentColor, setAccentColor] = useState("#3b82f6");
  
  useEffect(() => {
    const updateColor = () => {
      setAccentColor(getThemeColor("--color-accent-blue"));
    };
    
    // Update on mount
    updateColor();
    
    // Update periodically to catch theme changes
    const interval = setInterval(updateColor, 100);
    
    return () => clearInterval(interval);
  }, []);

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

  // Handle long press - logs to console
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
      <div className="neumorphic-button off">
        <svg className="horseshoe-progress" viewBox="0 0 200 200">
          {/* Empty SVG for structure consistency */}
        </svg>
        <span className="button-text" style={{ display: "flex", alignItems: "center", justifyContent: "center", width: "100%", height: "100%", position: "absolute", top: 0, left: 0 }}>
          <i className="fas fa-cog" style={{ color: accentColor, fontSize: "28px", display: "flex", alignItems: "center", justifyContent: "center" }}></i>
        </span>
      </div>
    </div>
  );
};
