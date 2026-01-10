/**
 * TableCard Component
 * Displays and controls table lift (up/down buttons)
 * Simplified: Single click/touch triggers auto movement
 */

import { useRef } from "react";

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
  
  // Debounce and touch tracking to prevent double triggers
  const lastActionTime = useRef(0);
  const touchStartPos = useRef({ x: 0, y: 0 });
  const hasTriggeredTouch = useRef(false); // Track if touch event already triggered action
  const DEBOUNCE_DELAY = 300; // ms - prevent rapid double clicks
  const MAX_TOUCH_MOVE = 15; // pixels - max movement to consider it a tap (not drag)
  
  // Determine button class and color
  let buttonClass = "neumorphic-button";
  let iconColor = "#9ca3af"; // Gray when inactive
  
  if (isActive) {
    buttonClass += " on";
    iconColor = "#3b82f6"; // Blue when active
  } else {
    buttonClass += " off";
  }
  
  // Common action handler - with debouncing
  const triggerAction = () => {
    // Debounce: prevent rapid multiple actions
    const now = Date.now();
    if (now - lastActionTime.current < DEBOUNCE_DELAY) {
      return; // Ignore rapid actions
    }
    lastActionTime.current = now;
    
    if (onClick) {
      onClick();
    }
  };
  
  // Handle click (desktop) - with protection against touch events
  const handleClick = (e) => {
    if (disabled) return;
    
    // If touch event already triggered, ignore this click (prevent double trigger)
    if (hasTriggeredTouch.current) {
      hasTriggeredTouch.current = false; // Reset for next interaction
      return;
    }
    
    e.preventDefault();
    e.stopPropagation();
    triggerAction();
  };
  
  // Handle touch start - track position
  const handleTouchStart = (e) => {
    if (disabled) return;
    hasTriggeredTouch.current = false; // Reset flag
    const touch = e.touches[0];
    if (touch) {
      touchStartPos.current = { x: touch.clientX, y: touch.clientY };
    }
  };
  
  // Handle touch end - trigger action if it was a tap (not drag)
  const handleTouchEnd = (e) => {
    if (disabled) return;
    e.preventDefault();
    e.stopPropagation();
    
    // Check if it was a drag or swipe (movement > threshold)
    if (e.changedTouches && e.changedTouches.length > 0) {
      const touch = e.changedTouches[0];
      const deltaX = Math.abs(touch.clientX - touchStartPos.current.x);
      const deltaY = Math.abs(touch.clientY - touchStartPos.current.y);
      
      // If moved too much, it was a drag - ignore
      if (deltaX > MAX_TOUCH_MOVE || deltaY > MAX_TOUCH_MOVE) {
        return;
      }
    }
    
    // Mark that touch event triggered action (to prevent onClick from also firing)
    hasTriggeredTouch.current = true;
    
    // It was a tap - trigger action
    triggerAction();
  };
  
  // Determine arrow direction
  const arrowPoints = direction === "up" 
    ? "50,20 45,35 50,30 55,35"  // Arrow pointing up
    : "50,80 45,65 50,70 55,65"; // Arrow pointing down
  
  return (
    <div 
      className={`led-card ${disabled ? "disabled" : ""}`}
      onClick={handleClick}
      onTouchStart={handleTouchStart}
      onTouchEnd={handleTouchEnd}
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

