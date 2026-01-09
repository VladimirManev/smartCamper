/**
 * TableCard Component
 * Displays and controls table lift (up/down buttons)
 */

import { useRef } from "react";

/**
 * TableCard component
 * @param {Object} props - Component props
 * @param {string} props.name - Display name (e.g., "Up" or "Down")
 * @param {string} props.direction - Direction: "up" or "down"
 * @param {Object} props.tableState - Table state object { direction }
 * @param {Function} props.onMouseDown - Mouse down handler (hold to move)
 * @param {Function} props.onMouseUp - Mouse up handler (stop)
 * @param {Function} props.onDoubleClick - Double click handler (auto move 5s)
 * @param {boolean} props.disabled - Whether the table control is disabled/offline
 */
export const TableCard = ({ 
  name, 
  direction, 
  tableState, 
  onMouseDown, 
  onMouseUp, 
  onDoubleClick,
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
  
  // Double-click detection
  const lastClickTime = useRef(0);
  const clickTimer = useRef(null);
  
  const handleClick = (e) => {
    if (disabled) return;
    
    const currentTime = Date.now();
    const timeSinceLastClick = currentTime - lastClickTime.current;
    
    // Clear previous single-click timer
    if (clickTimer.current) {
      clearTimeout(clickTimer.current);
      clickTimer.current = null;
    }
    
    // Check for double-click (within 500ms)
    if (timeSinceLastClick < 500 && lastClickTime.current > 0) {
      // Double-click detected
      lastClickTime.current = 0;
      if (onDoubleClick) {
        onDoubleClick();
      }
    } else {
      // Single click - wait for potential second click
      lastClickTime.current = currentTime;
      clickTimer.current = setTimeout(() => {
        // Single click confirmed (no second click within timeout)
        clickTimer.current = null;
        lastClickTime.current = 0;
      }, 500);
    }
  };
  
  // Handle mouse down (hold to move)
  const handleMouseDown = (e) => {
    if (disabled) return;
    e.preventDefault();
    if (onMouseDown) {
      onMouseDown();
    }
  };
  
  // Handle mouse up (stop)
  const handleMouseUp = (e) => {
    if (disabled) return;
    e.preventDefault();
    if (onMouseUp) {
      onMouseUp();
    }
  };
  
  // Handle touch events (for mobile)
  const handleTouchStart = (e) => {
    if (disabled) return;
    e.preventDefault();
    if (onMouseDown) {
      onMouseDown();
    }
  };
  
  const handleTouchEnd = (e) => {
    if (disabled) return;
    e.preventDefault();
    if (onMouseUp) {
      onMouseUp();
    }
  };
  
  // Determine arrow direction
  const arrowPoints = direction === "up" 
    ? "50,20 45,35 50,30 55,35"  // Arrow pointing up
    : "50,80 45,65 50,70 55,65"; // Arrow pointing down
  
  return (
    <div 
      className={`led-card ${disabled ? "disabled" : ""}`}
      onMouseDown={handleMouseDown}
      onMouseUp={handleMouseUp}
      onMouseLeave={handleMouseUp} // Stop if mouse leaves while holding
      onTouchStart={handleTouchStart}
      onTouchEnd={handleTouchEnd}
      onClick={handleClick}
    >
      <p className="led-name">{name}</p>
      <div className={buttonClass}>
        <svg className="table-icon" viewBox="0 0 100 100" width="100" height="100">
          {/* Table (rectangle) */}
          <rect
            x="20"
            y="40"
            width="60"
            height="20"
            rx="2"
            fill={iconColor}
            stroke={iconColor}
            strokeWidth="2"
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

