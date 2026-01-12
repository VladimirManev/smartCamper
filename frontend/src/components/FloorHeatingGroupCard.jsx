/**
 * FloorHeatingGroupCard Component
 * Group card that displays as a floor heating card in OFF state
 * Opens a modal with all floor heating cards when long pressed
 */

import { useLongPress } from "../hooks/useLongPress";

/**
 * FloorHeatingGroupCard component
 * @param {Object} props - Component props
 * @param {string} props.name - Display name (e.g., "Floor Heating")
 * @param {Function} props.onLongPress - Long press handler function (opens modal)
 * @param {boolean} props.disabled - Whether the control is disabled/offline
 */
export const FloorHeatingGroupCard = ({ name, onLongPress, disabled = false }) => {
  // Always show as OFF state for group card
  const isOff = true;
  const isTempControl = false;

  // Button class - always OFF state
  const buttonClass = "neumorphic-button off";

  // Generate unique gradient ID based on name
  const gradientId = `gradient-${name.toLowerCase().replace(/\s+/g, "-")}-heating-group`;

  // Handle click - placeholder (does nothing for now)
  const handleClick = () => {
    if (!disabled) {
      console.log("Floor Heating Group card clicked");
    }
  };

  // Handle long press
  const handleLongPress = () => {
    if (!disabled && onLongPress) {
      onLongPress();
    }
  };

  // Long press handlers
  const longPressHandlers = useLongPress(handleLongPress, handleClick);

  return (
    <div className={`led-card floor-heating-card ${disabled ? "disabled" : ""}`} {...longPressHandlers}>
      <p className="led-name">{name}</p>
      <div className={buttonClass}>
        <svg className="horseshoe-progress" viewBox="0 0 200 200">
          <defs>
            <linearGradient id={gradientId} x1="0%" y1="0%" x2="0%" y2="100%">
              <stop offset="0%" stopColor="#3b82f6" />
              <stop offset="100%" stopColor="#2563eb" />
            </linearGradient>
          </defs>
          {/* No circle shown in OFF state */}
        </svg>
        <span className="button-text">
          <span className="heating-symbol">
            <svg width="20" height="20" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
              {/* Three vertical wavy lines pointing up - heating symbol (shorter) */}
              <path d="M 8 18 Q 6 15, 8 12 T 8 6" stroke="currentColor" strokeWidth="2.5" strokeLinecap="round" fill="none"/>
              <path d="M 12 18 Q 10 15, 12 12 T 12 6" stroke="currentColor" strokeWidth="2.5" strokeLinecap="round" fill="none"/>
              <path d="M 16 18 Q 14 15, 16 12 T 16 6" stroke="currentColor" strokeWidth="2.5" strokeLinecap="round" fill="none"/>
            </svg>
          </span>
        </span>
      </div>
    </div>
  );
};

