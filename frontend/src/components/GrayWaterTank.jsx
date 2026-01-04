/**
 * GrayWaterTank Component
 * Displays gray water level visualization
 */

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

  return (
    <div className={`sensor-card water-tank-card ${disabled ? "disabled" : ""}`}>
      <p className="water-tank-label">Gray Water</p>
      <div className="water-tank-container">
        <svg
          className="water-tank"
          viewBox="0 0 200 200"
          preserveAspectRatio="xMidYMid meet"
        >
          {/* Tank outline */}
          <rect
            x="50"
            y="20"
            width="100"
            height="160"
            fill="none"
            stroke="#3b82f6"
            strokeWidth="4"
            rx="6"
          />

          {/* Water fill - fills from bottom up */}
          {displayLevel !== null && displayLevel !== undefined && (
            <rect
              x="54"
              y={180 - (displayLevel / 100) * 160}
              width="92"
              height={(displayLevel / 100) * 160}
              fill="url(#grayWaterGradient)"
              rx="4"
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
              <stop offset="0%" stopColor="#64748b" stopOpacity="0.8" />
              <stop offset="50%" stopColor="#475569" stopOpacity="0.9" />
              <stop offset="100%" stopColor="#334155" stopOpacity="1" />
            </linearGradient>
          </defs>

          {/* Water level indicator lines */}
          <line
            x1="44"
            y1="60"
            x2="50"
            y2="60"
            stroke="#3b82f6"
            strokeWidth="2"
            opacity="0.5"
          />
          <line
            x1="44"
            y1="100"
            x2="50"
            y2="100"
            stroke="#3b82f6"
            strokeWidth="2"
            opacity="0.5"
          />
          <line
            x1="44"
            y1="140"
            x2="50"
            y2="140"
            stroke="#3b82f6"
            strokeWidth="2"
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
              fill="#f5f5f5"
              fontWeight="600"
            >
              {temperature.toFixed(1)}°C
            </text>
          )}
        </svg>
      </div>
    </div>
  );
};

