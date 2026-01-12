/**
 * TableGroupCard Component
 * Group card that displays as a table card in OFF state
 * Opens a modal with all table cards when long pressed
 */

import { useLongPress } from "../hooks/useLongPress";

/**
 * TableGroupCard component
 * @param {Object} props - Component props
 * @param {string} props.name - Display name (e.g., "Table")
 * @param {Function} props.onClick - Click handler function (opens modal)
 * @param {boolean} props.disabled - Whether the control is disabled/offline
 */
export const TableGroupCard = ({ name, onClick, disabled = false }) => {
  // Always show as OFF state for group card
  const buttonClass = "neumorphic-button off";
  const iconColor = "#3b82f6"; // Blue for group card (like active state)

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
      console.log("Table Group card long pressed");
    }
  };

  // Long press handlers
  const longPressHandlers = useLongPress(handleLongPress, handleClick);

  return (
    <div className={`led-card ${disabled ? "disabled" : ""}`} {...longPressHandlers}>
      <p className="led-name">{name}</p>
      <div className={buttonClass}>
        <svg className="table-icon" viewBox="0 0 100 100" width="100" height="100">
          {/* Table (horizontal line) */}
          <line
            x1="20"
            y1="50"
            x2="80"
            y2="50"
            stroke={iconColor}
            strokeWidth="5"
            strokeLinecap="round"
          />
          {/* Arrow pointing up */}
          <polygon
            points="50,20 45,35 50,30 55,35"
            fill={iconColor}
            stroke={iconColor}
            strokeWidth="1.5"
            strokeLinejoin="round"
          />
          {/* Arrow pointing down */}
          <polygon
            points="50,80 45,65 50,70 55,65"
            fill={iconColor}
            stroke={iconColor}
            strokeWidth="1.5"
            strokeLinejoin="round"
          />
        </svg>
      </div>
    </div>
  );
};

