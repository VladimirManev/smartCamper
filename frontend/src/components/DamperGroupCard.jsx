/**
 * DamperGroupCard Component
 * Group card that displays as a damper card in OFF/closed state
 * Opens a modal with all damper cards when long pressed
 */

import { useLongPress } from "../hooks/useLongPress";

/**
 * DamperGroupCard component
 * @param {Object} props - Component props
 * @param {string} props.name - Display name (e.g., "Dampers")
 * @param {Function} props.onClick - Click handler function (opens modal)
 * @param {boolean} props.disabled - Whether the control is disabled/offline
 */
export const DamperGroupCard = ({ name, onClick, disabled = false }) => {
  // Always show as closed (OFF) state for group card
  const angle = 0;
  const isClosed = true;

  // Button class - always OFF state (no glow)
  const buttonClass = "neumorphic-button off";
  const damperColor = "var(--color-accent-blue)"; // Blue for group card (like active state)

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
      console.log("Damper Group card long pressed");
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
          
          {/* Damper blade (rotating line) - closed position (0°) */}
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

