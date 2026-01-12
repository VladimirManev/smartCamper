/**
 * TableCard Component
 * Displays and controls table lift (up/down buttons)
 * Supports two modes:
 * - onClick: Single click/touch triggers auto movement (5 seconds)
 * - onHoldStart/onHoldEnd: Hold to move continuously, release to stop
 */

import { useRef, useEffect } from "react";

/**
 * TableCard component
 * @param {Object} props - Component props
 * @param {string} props.name - Display name (e.g., "Up" or "Down")
 * @param {string} props.direction - Direction: "up" or "down"
 * @param {Object} props.tableState - Table state object { direction, autoMoving }
 * @param {Function} props.onClick - Click handler (auto move in direction or stop if already moving) - for auto mode
 * @param {Function} props.onHoldStart - Hold start handler (start continuous movement) - for hold mode
 * @param {Function} props.onHoldEnd - Hold end handler (stop movement) - for hold mode
 * @param {boolean} props.isAuto - Whether to show double arrows (for auto mode)
 * @param {boolean} props.disabled - Whether the table control is disabled/offline
 */
export const TableCard = ({ 
  name, 
  direction, 
  tableState, 
  onClick,
  onHoldStart,
  onHoldEnd,
  isAuto = false,
  disabled = false 
}) => {
  // Determine if this is hold mode or click mode
  const isHoldMode = onHoldStart && onHoldEnd;
  const directionState = tableState?.direction || "stopped";
  const isActive = directionState === direction;
  
  // Hold mode state
  const isHoldingRef = useRef(false);
  const holdTimeoutRef = useRef(null);
  
  // Click mode state
  const lastActionTime = useRef(0);
  const touchStartPos = useRef({ x: 0, y: 0 });
  const hasTriggeredTouch = useRef(false);
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
  
  // HOLD MODE HANDLERS
  const handleHoldStart = (e) => {
    if (disabled || !isHoldMode) return;
    e.preventDefault();
    e.stopPropagation();
    
    if (isHoldingRef.current) return; // Already holding
    
    isHoldingRef.current = true;
    if (onHoldStart) {
      onHoldStart();
    }
  };
  
  const handleHoldEnd = (e) => {
    if (disabled || !isHoldMode) return;
    e.preventDefault();
    e.stopPropagation();
    
    if (!isHoldingRef.current) return; // Not holding
    
    isHoldingRef.current = false;
    if (onHoldEnd) {
      onHoldEnd();
    }
  };
  
  // CLICK MODE HANDLERS
  const triggerAction = () => {
    if (isHoldMode) return; // Don't trigger click action in hold mode
    
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
  
  const handleClick = (e) => {
    if (disabled || isHoldMode) return;
    
    // If touch event already triggered, ignore this click (prevent double trigger)
    if (hasTriggeredTouch.current) {
      hasTriggeredTouch.current = false; // Reset for next interaction
      return;
    }
    
    e.preventDefault();
    e.stopPropagation();
    triggerAction();
  };
  
  const handleTouchStart = (e) => {
    if (disabled) return;
    
    if (isHoldMode) {
      handleHoldStart(e);
    } else {
      hasTriggeredTouch.current = false; // Reset flag
      const touch = e.touches[0];
      if (touch) {
        touchStartPos.current = { x: touch.clientX, y: touch.clientY };
      }
    }
  };
  
  const handleTouchEnd = (e) => {
    if (disabled) return;
    e.preventDefault();
    e.stopPropagation();
    
    if (isHoldMode) {
      handleHoldEnd(e);
    } else {
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
    }
  };
  
  // Mouse handlers for hold mode
  const handleMouseDown = (e) => {
    if (disabled) return;
    if (isHoldMode) {
      handleHoldStart(e);
    }
  };
  
  const handleMouseUp = (e) => {
    if (disabled) return;
    if (isHoldMode) {
      handleHoldEnd(e);
    }
  };
  
  const handleMouseLeave = (e) => {
    if (disabled) return;
    if (isHoldMode && isHoldingRef.current) {
      handleHoldEnd(e);
    }
  };

  // Cleanup: stop movement if disabled while holding
  useEffect(() => {
    if (disabled && isHoldingRef.current && isHoldMode && onHoldEnd) {
      isHoldingRef.current = false;
      onHoldEnd();
    }
  }, [disabled, isHoldMode, onHoldEnd]);

  // Global event listeners to catch mouse/touch release outside element
  useEffect(() => {
    if (!isHoldMode || !isHoldingRef.current) return;

    const handleGlobalMouseUp = (e) => {
      if (isHoldingRef.current) {
        isHoldingRef.current = false;
        if (onHoldEnd) {
          onHoldEnd();
        }
      }
    };

    const handleGlobalTouchEnd = (e) => {
      if (isHoldingRef.current) {
        isHoldingRef.current = false;
        if (onHoldEnd) {
          onHoldEnd();
        }
      }
    };

    document.addEventListener('mouseup', handleGlobalMouseUp);
    document.addEventListener('touchend', handleGlobalTouchEnd);
    document.addEventListener('touchcancel', handleGlobalTouchEnd);

    return () => {
      document.removeEventListener('mouseup', handleGlobalMouseUp);
      document.removeEventListener('touchend', handleGlobalTouchEnd);
      document.removeEventListener('touchcancel', handleGlobalTouchEnd);
    };
  }, [isHoldMode, onHoldEnd]);
  
  // Determine arrow points - single arrow, larger for auto mode
  let arrowPoints1, arrowPoints2;
  if (direction === "up") {
    if (isAuto) {
      // Large single arrow up (2-3x bigger)
      arrowPoints1 = "50,10 35,40 50,30 65,40";  // Large arrow up
      arrowPoints2 = null;
    } else {
      // Single arrow up
      arrowPoints1 = "50,20 45,35 50,30 55,35";
      arrowPoints2 = null;
    }
  } else {
    if (isAuto) {
      // Large single arrow down (2-3x bigger)
      arrowPoints1 = "50,90 35,60 50,70 65,60";  // Large arrow down
      arrowPoints2 = null;
    } else {
      // Single arrow down - pointing down (tip at bottom, base at top)
      arrowPoints1 = "50,80 45,65 50,70 55,65";
      arrowPoints2 = null;
    }
  }
  
  return (
    <div 
      className={`led-card ${disabled ? "disabled" : ""}`}
      onClick={isHoldMode ? undefined : handleClick}
      onMouseDown={handleMouseDown}
      onMouseUp={handleMouseUp}
      onMouseLeave={handleMouseLeave}
      onTouchStart={handleTouchStart}
      onTouchEnd={handleTouchEnd}
      onTouchCancel={isHoldMode ? handleHoldEnd : undefined}
    >
      <p className="led-name">{name}</p>
      <div className={buttonClass} style={{ display: 'flex', justifyContent: 'center', alignItems: 'center' }}>
        <svg className="table-icon" viewBox="0 0 100 100" width="100" height="100" style={{ display: 'block' }}>
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
          {/* Arrow(s) - single or double (stacked) based on isAuto */}
          <polygon
            points={arrowPoints1}
            fill={iconColor}
            stroke={iconColor}
            strokeWidth="1.5"
            strokeLinejoin="round"
          />
          {arrowPoints2 && (
            <polygon
              points={arrowPoints2}
              fill={iconColor}
              stroke={iconColor}
              strokeWidth="1.5"
              strokeLinejoin="round"
            />
          )}
        </svg>
      </div>
    </div>
  );
};

