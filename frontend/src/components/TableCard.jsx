/**
 * TableCard Component
 * Displays and controls table lift (up/down buttons)
 * Simplified: Single click/touch triggers auto movement
 */

/**
 * TableCard component
 * @param {Object} props - Component props
 * @param {string} props.name - Display name (e.g., "Up" or "Down")
 * @param {string} props.direction - Direction: "up" or "down"
 * @param {Object} props.tableState - Table state object { direction, autoMoving }
 * @param {Function} props.onClick - Click handler (auto move in direction or stop if already moving)
 * @param {boolean} props.disabled - Whether the table control is disabled/offline
 */
export const TableCard = ({ 
  name, 
  direction, 
  tableState, 
  onClick,
  disabled = false 
}) => {
  const directionState = tableState?.direction || "stopped";
  const isActive = directionState === direction;
  
  // Determine button class and color
  let buttonClass = "neumorphic-button";
  let iconColor = "#9ca3af"; // Gray when inactive
  
  if (isActive) {
    buttonClass += " on";
    iconColor = "#3b82f6"; // Blue when active
  } else {
    buttonClass += " off";
  }
  
  // Handle click/touch - simplified: just call onClick handler
  const handleClick = (e) => {
    if (disabled) return;
    e.preventDefault();
    e.stopPropagation();
    
    if (onClick) {
      onClick();
    }
  };
  
  // Determine arrow direction
  const arrowPoints = direction === "up" 
    ? "50,20 45,35 50,30 55,35"  // Arrow pointing up
    : "50,80 45,65 50,70 55,65"; // Arrow pointing down
  
  return (
    <div 
      className={`led-card ${disabled ? "disabled" : ""}`}
      onClick={handleClick}
      onTouchStart={handleClick}
    >
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
            strokeWidth="4"
            strokeLinecap="round"
          />
          {/* Arrow (up or down) */}
          <polygon
            points={arrowPoints}
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

