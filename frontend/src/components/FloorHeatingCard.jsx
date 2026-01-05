/**
 * FloorHeatingCard Component
 * Displays and controls a single floor heating circle
 */

/**
 * FloorHeatingCard component
 * @param {Object} props - Component props
 * @param {string} props.name - Display name (e.g., "Central 1")
 * @param {Object} props.circle - Circle state object { mode, relay, temperature, error? }
 * @param {Function} props.onClick - Click handler function
 * @param {boolean} props.disabled - Whether the control is disabled/offline
 */
export const FloorHeatingCard = ({ name, circle, onClick, disabled = false }) => {
  const mode = circle?.mode || "OFF";
  const relay = circle?.relay || "OFF";
  const temperature = circle?.temperature;
  const hasError = circle?.error || false;
  
  const isOff = mode === "OFF";
  const isTempControl = mode === "TEMP_CONTROL";
  const isRelayOn = relay === "ON";

  // Determine button class based on state
  let buttonClass = "neumorphic-button";
  if (hasError) {
    buttonClass += " error";  // Error state - red glow
  } else if (isTempControl && isRelayOn) {
    buttonClass += " on";  // TEMP_CONTROL + relay ON - red temperature
  } else if (isTempControl && !isRelayOn) {
    buttonClass += " temp-control-off";  // TEMP_CONTROL + relay OFF - blue temperature
  } else {
    buttonClass += " off";  // OFF mode
  }

  // Generate unique gradient ID based on name
  const gradientId = `gradient-${name.toLowerCase().replace(/\s+/g, "-")}-heating`;

  // Handle click - don't do anything if disabled
  const handleClick = () => {
    if (!disabled && onClick) {
      onClick();
    }
  };

  return (
    <div className={`led-card ${disabled ? "disabled" : ""}`} onClick={handleClick}>
      <p className="led-name">{name}</p>
      <div className={buttonClass}>
        <svg className="horseshoe-progress" viewBox="0 0 200 200">
          <defs>
            <linearGradient id={gradientId} x1="0%" y1="0%" x2="0%" y2="100%">
              <stop offset="0%" stopColor="#3b82f6" />
              <stop offset="100%" stopColor="#2563eb" />
            </linearGradient>
          </defs>
          {/* Closed circle - show if in TEMP_CONTROL mode (relay ON or OFF) */}
          {isTempControl && (
            <circle
              className="horseshoe-fill"
              cx="100"
              cy="100"
              r="80"
              fill="none"
              stroke={`url(#${gradientId})`}
              strokeWidth="8"
              strokeLinecap="round"
            />
          )}
        </svg>
        <span className="button-text">
          {hasError ? (
            <span className="error-icon">!</span>
          ) : isOff ? (
            <span className="heating-icon">🔥</span>
          ) : (
            <>
              {temperature !== null && temperature !== undefined ? Math.round(temperature) : "--"}°
              {isRelayOn && <span className="status-indicator"></span>}
            </>
          )}
        </span>
      </div>
    </div>
  );
};

