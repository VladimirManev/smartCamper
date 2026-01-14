/**
 * FloorHeatingCard Component
 * Displays and controls a single floor heating circle
 */

import { useLongPress } from "../hooks/useLongPress";

/**
 * FloorHeatingCard component
 * @param {Object} props - Component props
 * @param {string} props.name - Display name (e.g., "Central 1")
 * @param {Object} props.circle - Circle state object { mode, relay, temperature, error? }
 * @param {Function} props.onClick - Click handler function
 * @param {Function} props.onLongPress - Long press handler function (opens modal)
 * @param {boolean} props.disabled - Whether the control is disabled/offline
 */
export const FloorHeatingCard = ({ name, circle, onClick, onLongPress, disabled = false }) => {
  // Get theme colors
  const [accentBlue, setAccentBlue] = useState("#3b82f6");
  const [accentBlueDark, setAccentBlueDark] = useState("#2563eb");
  
  useEffect(() => {
    setAccentBlue(getThemeColor("--color-accent-blue"));
    setAccentBlueDark(getThemeColor("--color-accent-blue-dark"));
  }, []);
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
              <stop offset="0%" stopColor={accentBlue} />
              <stop offset="100%" stopColor={accentBlueDark} />
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
            <span className="heating-symbol">
              <svg width="20" height="20" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
                {/* Three vertical wavy lines pointing up - heating symbol (shorter) */}
                <path d="M 8 18 Q 6 15, 8 12 T 8 6" stroke="currentColor" strokeWidth="2.5" strokeLinecap="round" fill="none"/>
                <path d="M 12 18 Q 10 15, 12 12 T 12 6" stroke="currentColor" strokeWidth="2.5" strokeLinecap="round" fill="none"/>
                <path d="M 16 18 Q 14 15, 16 12 T 16 6" stroke="currentColor" strokeWidth="2.5" strokeLinecap="round" fill="none"/>
              </svg>
            </span>
          ) : (
            <span className="temperature-display">
              {temperature !== null && temperature !== undefined ? Math.round(temperature) : "--"}°
            </span>
          )}
        </span>
      </div>
    </div>
  );
};

