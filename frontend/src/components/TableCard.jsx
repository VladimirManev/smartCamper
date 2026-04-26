/**
 * TableCard Component
 * Displays and controls table lift (up/down buttons)
 * Supports two modes:
 * - onClick: Single click/touch triggers auto movement (5 seconds)
 * - onHoldStart/onHoldEnd: Hold to move continuously, release to stop
 */

import { useRef, useEffect } from "react";
import { Card } from "./Card";

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
  
  // Auto: double chevrons; hold: single chevron (viewBox 0 0 100 100). dy centers glyph vertically at y=50.
  let arrowPoints1;
  let arrowPoints2 = null;
  let arrowCenterDy = 0;
  if (direction === "up") {
    if (isAuto) {
      arrowPoints1 = "50,12 39,30 50,22 61,30";
      arrowPoints2 = "50,26 39,44 50,36 61,44";
      arrowCenterDy = 22; /* content ~y12–44 → center 28 */
    } else {
      arrowPoints1 = "50,16 38,48 50,36 62,48";
      arrowCenterDy = 18; /* ~y16–48 → center 32 */
    }
  } else if (isAuto) {
    arrowPoints1 = "50,70 39,58 50,62 61,58";
    arrowPoints2 = "50,88 39,72 50,80 61,72";
    arrowCenterDy = -23; /* ~y58–88 → center 73 */
  } else {
    arrowPoints1 = "50,84 38,52 50,64 62,52";
    arrowCenterDy = -18; /* ~y52–84 → center 68 */
  }

  // Table icon SVG
  const tableIcon = (
    <svg className="table-icon" viewBox="0 0 100 100">
      <g transform={`translate(0 ${arrowCenterDy})`}>
        <polygon
          points={arrowPoints1}
          fill="currentColor"
          stroke="currentColor"
          strokeWidth="1.25"
          strokeLinejoin="round"
        />
        {arrowPoints2 && (
          <polygon
            points={arrowPoints2}
            fill="currentColor"
            stroke="currentColor"
            strokeWidth="1.25"
            strokeLinejoin="round"
          />
        )}
      </g>
    </svg>
  );

  // Card reads iconState → class icon-${iconState}; "gray" is only 0.4 opacity — use inactive for idle arrows
  const buttonState = isActive ? "on" : "off";
  const iconState = isActive ? "active" : "inactive";

  // Custom handlers for hold mode (includes global event listeners)
  const customHandlers = isHoldMode ? {
    onMouseDown: handleMouseDown,
    onMouseUp: handleMouseUp,
    onMouseLeave: handleMouseLeave,
    onTouchStart: handleTouchStart,
    onTouchEnd: handleTouchEnd,
    onTouchCancel: handleHoldEnd,
  } : {
    onClick: handleClick,
    onTouchStart: handleTouchStart,
    onTouchEnd: handleTouchEnd,
  };

  // Global event listeners for hold mode
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

  // Cleanup: stop movement if disabled while holding
  useEffect(() => {
    if (disabled && isHoldingRef.current && isHoldMode && onHoldEnd) {
      isHoldingRef.current = false;
      onHoldEnd();
    }
  }, [disabled, isHoldMode, onHoldEnd]);
  
  return (
    <Card
      name={name}
      icon={tableIcon}
      buttonState={buttonState}
      iconState={iconState}
      onClick={isHoldMode ? undefined : onClick}
      onHoldStart={onHoldStart}
      onHoldEnd={onHoldEnd}
      disabled={disabled}
      mode={isHoldMode ? "hold" : "click"}
      customHandlers={customHandlers}
    />
  );
};

