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
  
  // Track if double-click was detected (from onDoubleClick event)
  const doubleClickDetected = useRef(false);
  const mouseDownTimeout = useRef(null);
  
  // Handle double-click (use native React event)
  const handleDoubleClick = (e) => {
    if (disabled) return;
    e.preventDefault();
    e.stopPropagation();
    
    // Mark double-click detected
    doubleClickDetected.current = true;
    
    // Cancel any pending mouseDown action
    if (mouseDownTimeout.current) {
      clearTimeout(mouseDownTimeout.current);
      mouseDownTimeout.current = null;
    }
    
    // Call double-click handler
    if (onDoubleClick) {
      onDoubleClick();
    }
    
    // Reset flag after delay
    setTimeout(() => {
      doubleClickDetected.current = false;
    }, 200);
  };
  
  // Handle mouse down (hold to move) - with delay to allow double-click detection
  const handleMouseDown = (e) => {
    if (disabled) return;
    e.preventDefault();
    
    // CRITICAL: If auto-moving, stop immediately on any button press
    if (tableState?.autoMoving) {
      // Cancel any pending timeout
      if (mouseDownTimeout.current) {
        clearTimeout(mouseDownTimeout.current);
        mouseDownTimeout.current = null;
      }
      // Stop movement immediately
      if (onMouseUp) {
        onMouseUp();
      }
      return;
    }
    
    // Cancel any existing timeout
    if (mouseDownTimeout.current) {
      clearTimeout(mouseDownTimeout.current);
      mouseDownTimeout.current = null;
    }
    
    // Add delay (700ms) - longer than double-click detection timeout
    // This delay allows onDoubleClick to be called first if it's a double-click
    mouseDownTimeout.current = setTimeout(() => {
      // Only start movement if double-click was not detected
      if (!doubleClickDetected.current && onMouseDown) {
        onMouseDown();
      }
      mouseDownTimeout.current = null;
    }, 700); // 700ms delay - allows double-click event to fire first
  };
  
  // Handle mouse up (stop)
  const handleMouseUp = (e) => {
    if (disabled) return;
    e.preventDefault();
    
    // Cancel pending mouseDown if exists
    if (mouseDownTimeout.current) {
      clearTimeout(mouseDownTimeout.current);
      mouseDownTimeout.current = null;
    }
    
    // Call stop handler
    if (onMouseUp) {
      onMouseUp();
    }
  };
  
  // Handle touch events (for mobile) - same logic as mouse
  const handleTouchStart = (e) => {
    if (disabled) return;
    e.preventDefault();
    
    // CRITICAL: If auto-moving, stop immediately on any button press
    if (tableState?.autoMoving) {
      if (mouseDownTimeout.current) {
        clearTimeout(mouseDownTimeout.current);
        mouseDownTimeout.current = null;
      }
      if (onMouseUp) {
        onMouseUp();
      }
      return;
    }
    
    if (mouseDownTimeout.current) {
      clearTimeout(mouseDownTimeout.current);
      mouseDownTimeout.current = null;
    }
    
    mouseDownTimeout.current = setTimeout(() => {
      if (!doubleClickDetected.current && onMouseDown) {
        onMouseDown();
      }
      mouseDownTimeout.current = null;
    }, 700);
  };
  
  const handleTouchEnd = (e) => {
    if (disabled) return;
    e.preventDefault();
    
    if (mouseDownTimeout.current) {
      clearTimeout(mouseDownTimeout.current);
      mouseDownTimeout.current = null;
    }
    
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
      onDoubleClick={handleDoubleClick}
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

