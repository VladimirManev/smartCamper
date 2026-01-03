/**
 * LEDCard Component
 * Displays and controls a single LED strip or relay
 */

import { getArcProgress } from "../utils/arcProgress";

/**
 * LEDCard component
 * @param {Object} props - Component props
 * @param {string} props.name - Display name (e.g., "Kitchen")
 * @param {Object} props.strip - Strip state object { state, brightness, mode? }
 * @param {Function} props.onClick - Click handler function
 * @param {string} props.type - Type: "strip" or "relay" (default: "strip")
 */
export const LEDCard = ({ name, strip, onClick, type = "strip" }) => {
  const isOn = strip?.state === "ON";
  const brightness = strip?.brightness || 0;
  const mode = strip?.mode;

  // Calculate arc progress for strips
  const arcLength = Math.PI * 80 * (270 / 180);
  const progress = type === "strip" ? getArcProgress(brightness, isOn) : 0;

  // Determine button class based on state
  let buttonClass = "neumorphic-button";
  if (type === "relay") {
    buttonClass += isOn ? " on" : " off";
  } else {
    // For strips with mode (Bathroom)
    if (mode === "AUTO") {
      buttonClass += " auto";
    } else {
      buttonClass += isOn ? " on" : " off";
    }
  }

  // Determine display text
  const displayText = mode === "AUTO" ? "AUTO" : strip?.state || "OFF";

  // Generate unique gradient ID based on name and type
  // Use a simple hash to ensure uniqueness
  const gradientId = `gradient-${name.toLowerCase().replace(/\s+/g, "-")}-${type}`;

  return (
    <div className="led-card" onClick={onClick}>
      <p className="led-name">{name}</p>
      <div className={buttonClass}>
        {type === "strip" ? (
          <svg className="horseshoe-progress" viewBox="0 0 200 200">
            <defs>
              <linearGradient id={gradientId} x1="0%" y1="0%" x2="0%" y2="100%">
                <stop offset="0%" stopColor="#00C6FF" />
                <stop offset="100%" stopColor="#00FF99" />
              </linearGradient>
            </defs>
            {/* Arc from 135° (start) to 45° (end) - fills according to brightness */}
            <path
              className="horseshoe-fill"
              d="M 43.4 156.6 A 80 80 0 1 1 156.6 156.6"
              fill="none"
              stroke={`url(#${gradientId})`}
              strokeWidth="8"
              strokeLinecap="round"
              strokeDasharray={`${progress} ${arcLength}`}
              strokeDashoffset="0"
              opacity={isOn && progress > 0 ? 1 : 0}
            />
          </svg>
        ) : (
          <svg className="horseshoe-progress" viewBox="0 0 200 200">
            <defs>
              <linearGradient id={gradientId} x1="0%" y1="0%" x2="0%" y2="100%">
                <stop offset="0%" stopColor="#00C6FF" />
                <stop offset="100%" stopColor="#00FF99" />
              </linearGradient>
            </defs>
            {/* Closed circle - if ON it exists, if OFF it doesn't */}
            {isOn && (
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
        )}
        <span className="button-text">{displayText}</span>
      </div>
    </div>
  );
};

