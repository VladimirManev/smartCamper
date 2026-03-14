/**
 * DamperCard Component
 * Displays and controls a single damper (air vent)
 */

import { memo } from "react";
import { Card } from "./Card";

/**
 * DamperCard component
 * @param {Object} props - Component props
 * @param {string} props.name - Display name (e.g., "Kitchen")
 * @param {Object} props.damper - Damper state object { angle }
 * @param {Function} props.onClick - Click handler function
 * @param {Function} props.onLongPress - Long press handler function (opens modal)
 * @param {boolean} props.disabled - Whether the damper control is disabled/offline
 */
const DamperCardComponent = ({ name, damper, onClick, onLongPress, disabled = false }) => {
  const angle = damper?.angle || 0;
  
  // Determine damper state: 0° (closed), 45° (half-open), 90° (open)
  const isOpen = angle === 90;
  const isHalfOpen = angle === 45;
  const isActive = isOpen || isHalfOpen;
  
  // Determine button state and icon state
  const buttonState = isActive ? "on" : "off";
  const iconState = isActive ? "active" : "gray";

  // Damper icon SVG
  const damperIcon = (
    <svg className="damper-icon" viewBox="0 0 100 100">
      {/* Two horizontal lines (air duct) - shorter */}
      <line
        x1="30"
        y1="30"
        x2="70"
        y2="30"
        stroke="currentColor"
        strokeWidth="4"
        strokeLinecap="round"
      />
      <line
        x1="30"
        y1="70"
        x2="70"
        y2="70"
        stroke="currentColor"
        strokeWidth="4"
        strokeLinecap="round"
      />
      
      {/* Air flow arrow at left edge (pointing left) - wide angle with tail */}
      <line
        x1="20"
        y1="50"
        x2="30"
        y2="50"
        stroke="currentColor"
        strokeWidth="2"
        strokeLinecap="round"
      />
      <path
        d="M 30 50 L 25 45 M 30 50 L 25 55"
        stroke="currentColor"
        strokeWidth="2.5"
        strokeLinecap="round"
        fill="none"
      />
      
      {/* Damper blade (rotating line) - shorter, doesn't touch outer lines */}
      <g transform={`translate(50, 50) rotate(${angle}) translate(-50, -50)`}>
        <line
          x1="50"
          y1="40"
          x2="50"
          y2="60"
          stroke="currentColor"
          strokeWidth="6"
          strokeLinecap="round"
        />
      </g>
    </svg>
  );

  return (
    <Card
      name={name}
      icon={damperIcon}
      buttonState={buttonState}
      iconState={iconState}
      onClick={onClick}
      onLongPress={onLongPress}
      disabled={disabled}
    />
  );
};

// Memoize component to prevent unnecessary re-renders
export const DamperCard = memo(DamperCardComponent);
