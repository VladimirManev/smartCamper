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
  const DEBOUNCE_DELAY = 300; // ms - prevent rapid double clicks
  
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
