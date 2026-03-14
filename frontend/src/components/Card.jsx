/**
 * Unified Card Component
 * Reusable card component for all card types
 * All colors are managed through CSS variables defined in themes
 */

import { useLongPress } from "../hooks/useLongPress";
import { useRef } from "react";

/**
 * Card component - unified component for all card types
 * @param {Object} props - Component props
 * @param {string} props.name - Display name (shown below card)
 * @param {ReactNode} props.icon - Icon element (SVG, img, or FontAwesome icon)
 * @param {string} props.buttonState - Button state: "on" | "off" | "auto" | "error" | "temp-control-off" (default: "off")
 * @param {string} props.iconState - Icon state: "active" | "inactive" | "gray" (default: "inactive")
 * @param {Function} props.onClick - Click handler function (called on single click/tap)
 * @param {Function} props.onLongPress - Long press handler function (called on long press, typically opens modal)
 * @param {Function} props.onHoldStart - Hold start handler (for continuous actions like TableCard)
 * @param {Function} props.onHoldEnd - Hold end handler (for continuous actions like TableCard)
 * @param {boolean} props.disabled - Whether the card is disabled/offline
 * @param {string} props.cardClass - Additional CSS classes for card wrapper (e.g., "floor-heating-card")
 * @param {ReactNode} props.children - Additional content inside button (e.g., progress arc, temperature display, text)
 * @param {Object} props.customHandlers - Custom event handlers object (if provided, overrides default handlers)
 * @param {string} props.mode - Interaction mode: "click" (default) | "hold" (for hold-to-activate like TableCard)
 */
export const Card = ({
  name,
  icon,
  buttonState = "off",
  iconState = "inactive",
  onClick,
  onLongPress,
  onHoldStart,
  onHoldEnd,
  disabled = false,
  cardClass = "",
  children = null,
  customHandlers = null,
  mode = "click",
}) => {
  // Determine button class
  const buttonClass = `neumorphic-button ${buttonState}`;
  
  // Determine icon class
  const iconClass = `icon-${iconState}`;
  
  // Check if this is hold mode
  const isHoldMode = mode === "hold" && onHoldStart && onHoldEnd;
  
  // Hold mode state
  const isHoldingRef = useRef(false);
  const holdTimeoutRef = useRef(null);
  
  // Click mode state
  const lastActionTime = useRef(0);
  const touchStartPos = useRef({ x: 0, y: 0 });
  const hasTriggeredTouch = useRef(false);
  const DEBOUNCE_DELAY = 300; // ms - prevent rapid double clicks
  const MAX_TOUCH_MOVE = 15; // pixels - max movement to consider it a tap (not drag)
  
  // Handle click - for click mode
  const handleClick = (e) => {
    if (disabled || isHoldMode) return;
    if (e) {
      e.preventDefault();
      e.stopPropagation();
    }
    
    // Debounce: prevent rapid multiple actions
    const now = Date.now();
    if (now - lastActionTime.current < DEBOUNCE_DELAY) {
      return;
    }
    lastActionTime.current = now;
    
    if (onClick) {
      onClick();
    }
  };
  
  // Handle long press - opens modal or custom action
  const handleLongPress = () => {
    if (disabled || isHoldMode) return;
    if (onLongPress) {
      onLongPress();
    }
  };
  
  // HOLD MODE HANDLERS
  const handleHoldStart = (e) => {
    if (disabled || !isHoldMode) return;
    if (e) {
      e.preventDefault();
      e.stopPropagation();
    }
    
    if (isHoldingRef.current) return;
    
    isHoldingRef.current = true;
    if (onHoldStart) {
      onHoldStart();
    }
  };
  
  const handleHoldEnd = (e) => {
    if (disabled || !isHoldMode) return;
    if (e) {
      e.preventDefault();
      e.stopPropagation();
    }
    
    if (!isHoldingRef.current) return;
    
    isHoldingRef.current = false;
    if (onHoldEnd) {
      onHoldEnd();
    }
  };
  
  // CLICK MODE HANDLERS (for touch devices)
  const handleTouchStart = (e) => {
    if (disabled || isHoldMode) return;
    
    const touch = e.touches[0];
    touchStartPos.current = { x: touch.clientX, y: touch.clientY };
    hasTriggeredTouch.current = false;
  };
  
  const handleTouchEnd = (e) => {
    if (disabled || isHoldMode || hasTriggeredTouch.current) return;
    
    const touch = e.changedTouches[0];
    const moveX = Math.abs(touch.clientX - touchStartPos.current.x);
    const moveY = Math.abs(touch.clientY - touchStartPos.current.y);
    
    if (moveX < MAX_TOUCH_MOVE && moveY < MAX_TOUCH_MOVE) {
      hasTriggeredTouch.current = true;
      handleClick(e);
    }
  };
  
  // Mouse handlers for hold mode
  const handleMouseDown = (e) => {
    if (isHoldMode) {
      handleHoldStart(e);
    }
  };
  
  const handleMouseUp = (e) => {
    if (isHoldMode) {
      handleHoldEnd(e);
    }
  };
  
  const handleMouseLeave = (e) => {
    if (isHoldMode) {
      handleHoldEnd(e);
    }
  };
  
  // Use custom handlers if provided, otherwise use default handlers
  if (customHandlers) {
    return (
      <div className={`led-card ${cardClass} ${disabled ? "disabled" : ""}`} {...customHandlers}>
        <p className="led-name">{name}</p>
        <div className={buttonClass}>
          {children}
          <span className="button-text">
            {icon && (
              <div className={`icon-container ${iconClass}`}>
                {icon}
              </div>
            )}
          </span>
        </div>
      </div>
    );
  }
  
  // Default handlers based on mode
  if (isHoldMode) {
    return (
      <div
        className={`led-card ${cardClass} ${disabled ? "disabled" : ""}`}
        onMouseDown={handleMouseDown}
        onMouseUp={handleMouseUp}
        onMouseLeave={handleMouseLeave}
        onTouchStart={handleHoldStart}
        onTouchEnd={handleHoldEnd}
        onTouchCancel={handleHoldEnd}
      >
        <p className="led-name">{name}</p>
        <div className={buttonClass}>
          {children}
          <span className="button-text">
            {icon && (
              <div className={`icon-container ${iconClass}`}>
                {icon}
              </div>
            )}
          </span>
        </div>
      </div>
    );
  }
  
  // Click mode - use long press hook
  const longPressHandlers = useLongPress(handleLongPress, handleClick);
  
  return (
    <div
      className={`led-card ${cardClass} ${disabled ? "disabled" : ""}`}
      {...longPressHandlers}
      onTouchStart={handleTouchStart}
      onTouchEnd={handleTouchEnd}
    >
      <p className="led-name">{name}</p>
      <div className={buttonClass}>
        {children}
        <span className="button-text">
          {icon && (
            <div className={`icon-container ${iconClass}`}>
              {icon}
            </div>
          )}
        </span>
      </div>
    </div>
  );
};
