/**
 * LevelingGroupCard Component
 * Group card for leveling functionality
 * Opens leveling modal when clicked
 */

import { useLongPress } from "../hooks/useLongPress";
import { useEffect, useState } from "react";
import { getThemeColor } from "../utils/getThemeColor";
import spiritLevelIcon from "../assets/spirit-level-tool-svgrepo-com.svg";

/**
 * LevelingGroupCard component
 * @param {Object} props - Component props
 * @param {string} props.name - Display name (e.g., "Leveling")
 * @param {Function} props.onClick - Click handler function (opens modal)
 * @param {boolean} props.disabled - Whether the control is disabled/offline
 */
export const LevelingGroupCard = ({ name, onClick, disabled = false }) => {
  // Get theme colors
  const [accentBlue, setAccentBlue] = useState("#3b82f6");
  const [accentBlueDark, setAccentBlueDark] = useState("#2563eb");
  const [accentColor, setAccentColor] = useState("#3b82f6");
  
  useEffect(() => {
    const blue = getThemeColor("--color-accent-blue");
    setAccentBlue(blue);
    setAccentBlueDark(getThemeColor("--color-accent-blue-dark"));
    setAccentColor(blue);
  }, []);
  // Button class - always OFF state (no glow)
  const buttonClass = "neumorphic-button off";

  // Generate unique gradient ID based on name
  const gradientId = `gradient-${name.toLowerCase().replace(/\s+/g, "-")}-leveling-group`;

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
    if (!disabled) {
      console.log("Leveling Group card long pressed");
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
        </svg>
        <span className="button-text">
          <span className="leveling-icon">
            <img 
              src={spiritLevelIcon} 
              alt="Level" 
              className="leveling-icon-img"
              style={{ 
                width: '36px', 
                height: '36px',
                filter: accentColor === "#475569" 
                  ? 'brightness(0) saturate(100%)' // Dark gray - no hue rotation needed
                  : 'brightness(0) saturate(100%) invert(45%) sepia(96%) saturate(2000%) hue-rotate(210deg) brightness(1) contrast(1)' // Blue
              }}
            />
          </span>
        </span>
      </div>
    </div>
  );
};
