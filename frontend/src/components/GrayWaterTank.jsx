/**
 * GrayWaterTank Component
 * Displays gray water level visualization
 */

/**
 * GrayWaterTank component
 * @param {Object} props - Component props
 * @param {number|null} props.level - Water level percentage (0-100)
 */
export const GrayWaterTank = ({ level }) => {
  return (
    <div className="sensor-card water-tank-card">
      <p className="water-tank-label">Gray Water</p>
      <div className="water-tank-container">
        <svg
          className="water-tank"
          viewBox="0 0 100 100"
          preserveAspectRatio="xMidYMid meet"
        >
          {/* Tank outline */}
          <rect
            x="25"
            y="10"
            width="50"
            height="80"
            fill="none"
            stroke="#b3e5b3"
            strokeWidth="2"
            rx="3"
          />

          {/* Water fill - fills from bottom up */}
          {level !== null && level !== undefined && (
            <rect
              x="27"
              y={90 - (level / 100) * 80}
              width="46"
              height={(level / 100) * 80}
              fill="url(#grayWaterGradient)"
              rx="2"
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
              <stop offset="0%" stopColor="#95a5a6" stopOpacity="0.8" />
              <stop offset="50%" stopColor="#7f8c8d" stopOpacity="0.9" />
              <stop offset="100%" stopColor="#5d6d7e" stopOpacity="1" />
            </linearGradient>
          </defs>

          {/* Water level indicator lines */}
          <line
            x1="22"
            y1="30"
            x2="25"
            y2="30"
            stroke="#b3e5b3"
            strokeWidth="1"
            opacity="0.5"
          />
          <line
            x1="22"
            y1="50"
            x2="25"
            y2="50"
            stroke="#b3e5b3"
            strokeWidth="1"
            opacity="0.5"
          />
          <line
            x1="22"
            y1="70"
            x2="25"
            y2="70"
            stroke="#b3e5b3"
            strokeWidth="1"
            opacity="0.5"
          />
        </svg>
      </div>
    </div>
  );
};

