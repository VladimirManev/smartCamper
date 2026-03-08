/**
 * GrayWaterTank Component
 * Displays gray water level visualization
 */

import { useEffect, useState } from "react";
import { getThemeColor } from "../utils/getThemeColor";

/**
 * GrayWaterTank component
 * @param {Object} props - Component props
 * @param {number|null} props.level - Water level percentage (0-100)
 * @param {number|null} props.temperature - Water temperature in Celsius
 * @param {boolean} props.disabled - Whether the sensor is disabled/offline
 */
export const GrayWaterTank = ({ level, temperature, disabled = false }) => {
  // When disabled, don't show water (empty tank)
  const displayLevel = disabled ? null : level;
  
  // Get theme colors
  const [accentBlue, setAccentBlue] = useState("#3b82f6");
  const [accentBlueDark, setAccentBlueDark] = useState("#2563eb");
  const [textPrimary, setTextPrimary] = useState("#f5f5f5");
  
  useEffect(() => {
    const updateColors = () => {
      setAccentBlue(getThemeColor("--color-accent-blue"));
      setAccentBlueDark(getThemeColor("--color-accent-blue-dark"));
      setTextPrimary(getThemeColor("--color-text-primary"));
    };
    
    // Update on mount
    updateColors();
    
    // Update periodically to catch theme changes (reduced frequency for better performance)
    const interval = setInterval(updateColors, 2000); // 2 seconds instead of 100ms
    
    return () => clearInterval(interval);
  }, []);

  return (
    <div className={`sensor-card water-tank-card ${disabled ? "disabled" : ""}`}>
      <p className="water-tank-label">Gray Water</p>
      <div className="water-tank-container">
        <svg
          className="water-tank"
          viewBox="0 0 200 200"
          preserveAspectRatio="xMidYMid meet"
        >
          {/* Tank outline - open at top (no top line) */}
          <path
            d="M 45 20 L 45 172 Q 45 180 53 180 L 147 180 Q 155 180 155 172 L 155 20"
            fill="none"
            stroke={accentBlue}
            strokeWidth="8"
            strokeLinecap="round"
            strokeLinejoin="round"
          />

          {/* Water fill - fills from bottom up */}
          {displayLevel !== null && displayLevel !== undefined && (
            <rect
              x="49"
              y={180 - (displayLevel / 100) * 160}
              width="102"
              height={(displayLevel / 100) * 160}
              fill="url(#grayWaterGradient)"
              rx="8"
              style={{
                transition: "y 0.5s ease, height 0.5s ease",
              }}
            />
          )}

          {/* Gray water gradient */}
          <defs>
            <linearGradient
              id="grayWaterGradient"
              x1="0%"
              y1="0%"
              x2="0%"
              y2="100%"
            >
              <stop offset="0%" stopColor={accentBlue} stopOpacity="0.6" />
              <stop offset="50%" stopColor={accentBlue} stopOpacity="0.7" />
              <stop offset="100%" stopColor={accentBlueDark} stopOpacity="0.8" />
            </linearGradient>
          </defs>

          {/* Water level indicator lines */}
          <line
            x1="44"
            y1="60"
            x2="50"
            y2="60"
            stroke={accentBlue}
            strokeWidth="3"
            opacity="0.5"
          />
          <line
            x1="44"
            y1="100"
            x2="50"
            y2="100"
            stroke={accentBlue}
            strokeWidth="3"
            opacity="0.5"
          />
          <line
            x1="44"
            y1="140"
            x2="50"
            y2="140"
            stroke={accentBlue}
            strokeWidth="3"
            opacity="0.5"
          />

          {/* Temperature display - centered in tank */}
          {temperature !== null && temperature !== undefined && !disabled && (
            <text
              x="100"
              y="100"
              textAnchor="middle"
              dominantBaseline="middle"
              className="water-tank-temperature"
              fill={textPrimary}
              fontWeight="600"
            >
              {temperature.toFixed(1)}°
            </text>
          )}
        </svg>
      </div>
    </div>
  );
};

