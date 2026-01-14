/**
 * LevelingGroupCard Component
 * Group card for leveling functionality
 * Opens leveling modal when clicked
 */

import { useLongPress } from "../hooks/useLongPress";

/**
 * LevelingGroupCard component
 * @param {Object} props - Component props
 * @param {string} props.name - Display name (e.g., "Leveling")
 * @param {Function} props.onClick - Click handler function (opens modal)
 * @param {boolean} props.disabled - Whether the control is disabled/offline
 */
export const LevelingGroupCard = ({ name, onClick, disabled = false }) => {
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
              <stop offset="0%" stopColor="#3b82f6" />
              <stop offset="100%" stopColor="#2563eb" />
            </linearGradient>
          </defs>
        </svg>
        <span className="button-text">
          <span className="leveling-icon">
            <svg width="20" height="20" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
              {/* Leveling icon - horizontal lines with bubble in center */}
              {/* Top line */}
              <line x1="2" y1="6" x2="10" y2="6" stroke="#3b82f6" strokeWidth="2.5" strokeLinecap="round"/>
              {/* Bottom line */}
              <line x1="2" y1="18" x2="10" y2="18" stroke="#3b82f6" strokeWidth="2.5" strokeLinecap="round"/>
              {/* Center bubble (level indicator) */}
              <circle cx="14" cy="12" r="6" stroke="#3b82f6" strokeWidth="2.5" fill="none"/>
              <circle cx="14" cy="12" r="2" fill="#3b82f6"/>
              {/* Right line */}
              <line x1="20" y1="12" x2="22" y2="12" stroke="#3b82f6" strokeWidth="2.5" strokeLinecap="round"/>
            </svg>
          </span>
        </span>
      </div>
    </div>
  );
};
