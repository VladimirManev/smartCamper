/**
 * DamperCard Component
 * Displays and controls a single damper (air vent)
 */

import { useLongPress } from "../hooks/useLongPress";

/**
 * DamperCard component
 * @param {Object} props - Component props
 * @param {string} props.name - Display name (e.g., "Kitchen")
 * @param {Object} props.damper - Damper state object { angle }
 * @param {Function} props.onClick - Click handler function
 * @param {Function} props.onLongPress - Long press handler function (opens modal)
 * @param {boolean} props.disabled - Whether the damper control is disabled/offline
 */
export const DamperCard = ({ name, damper, onClick, onLongPress, disabled = false }) => {
  const angle = damper?.angle || 0;
  
  // Determine damper state: 0° (closed), 45° (half-open), 90° (open)
  const isClosed = angle === 0;
  const isHalfOpen = angle === 45;
  const isOpen = angle === 90;
  
  // Determine button class and color
  let buttonClass = "neumorphic-button";
  let damperColor = "var(--color-accent-gray)"; // Gray for closed
  
  if (isOpen || isHalfOpen) {
    buttonClass += " on";
    damperColor = "var(--color-accent-blue)"; // Blue for open/half-open
  } else {
    buttonClass += " off";
  }

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
        <svg className="damper-icon" viewBox="0 0 100 100" width="100" height="100">
          {/* Two horizontal lines (air duct) - shorter */}
          <line
            x1="30"
            y1="30"
            x2="70"
            y2="30"
            stroke={damperColor}
            strokeWidth="4"
            strokeLinecap="round"
          />
          <line
            x1="30"
            y1="70"
            x2="70"
            y2="70"
            stroke={damperColor}
            strokeWidth="4"
            strokeLinecap="round"
          />
          
          {/* Air flow arrow at left edge (pointing left) - wide angle with tail */}
          <line
            x1="20"
            y1="50"
            x2="30"
            y2="50"
            stroke={damperColor}
            strokeWidth="2"
            strokeLinecap="round"
          />
          <path
            d="M 30 50 L 25 45 M 30 50 L 25 55"
            stroke={damperColor}
            strokeWidth="2.5"
            strokeLinecap="round"
            fill="none"
          />
          
          {/* Damper blade (rotating line) - shorter, doesn't touch outer lines */}
          <g transform={`translate(50, 50) rotate(${angle}) translate(-50, -50)`}>
            <line
              x1="50"
              y1="40"
              x2="50"
              y2="60"
              stroke={damperColor}
              strokeWidth="6"
              strokeLinecap="round"
            />
          </g>
        </svg>
      </div>
    </div>
  );
};

