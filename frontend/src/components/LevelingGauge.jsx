/**
 * LevelingGauge Component
 * Displays a circular gauge with horizontal line that rotates based on angle
 * Similar to clock design on main page
 */

/**
 * LevelingGauge component
 * @param {Object} props - Component props
 * @param {string} props.label - Label text (e.g., "Pitch" or "Roll")
 * @param {number|null} props.angle - Angle value in degrees (null if no data)
 * @param {string} props.axis - Axis name ("X" for pitch, "Y" for roll)
 */
export const LevelingGauge = ({ label, angle, axis }) => {
  // Clamp angle to reasonable range for display (-10° to +10°)
  const clampedAngle = angle !== null && angle !== undefined 
    ? Math.max(-10, Math.min(10, angle))
    : 0;
  
  // Rotation multiplier for more sensitive visualization
  const rotationMultiplier = 15;
  
  // Convert angle to rotation (negative because positive angle should rotate clockwise)
  // Multiply by coefficient for more visible rotation
  const rotation = -clampedAngle * rotationMultiplier;
  
  // Display value
  const displayValue = angle !== null && angle !== undefined 
    ? angle.toFixed(1)
    : "--";
  
  // SVG viewBox dimensions
  const viewBoxSize = 200;
  const center = viewBoxSize / 2;
  const outerRadius = 85;  // Outer circle radius
  const tickLength = 8;     // Length of tick marks extending outward
  
  // Generate tick marks every 10 degrees around full circle (0° to 350°)
  const tickMarks = [];
  for (let angle = 0; angle < 360; angle += 10) {
    tickMarks.push(angle);
  }
  
  return (
    <div className="leveling-gauge-container">
      <div className="leveling-gauge">
        <svg className="leveling-gauge-svg" viewBox={`0 0 ${viewBoxSize} ${viewBoxSize}`}>
          <defs>
            <linearGradient id={`leveling-gradient-${axis}`} x1="0%" y1="0%" x2="0%" y2="100%">
              <stop offset="0%" stopColor="#3b82f6" />
              <stop offset="100%" stopColor="#2563eb" />
            </linearGradient>
          </defs>
          
          {/* Outer circle (background) */}
          <circle
            cx={center}
            cy={center}
            r={outerRadius}
            fill="none"
            stroke="rgba(59, 130, 246, 0.2)"
            strokeWidth="2"
          />
          
          {/* Tick marks every 10 degrees around full circle - extending outward from outer circle */}
          {tickMarks.map((tickAngle) => {
            // Convert angle to radians (SVG: 0° is at 3 o'clock, positive is clockwise)
            // Adjust: subtract 90° so 0° is at top (12 o'clock)
            const angleRad = ((tickAngle - 90) * Math.PI) / 180;
            // Calculate start point (on outer circle) and end point (extending outward)
            const x1 = center + outerRadius * Math.cos(angleRad);
            const y1 = center + outerRadius * Math.sin(angleRad);
            const x2 = center + (outerRadius + tickLength) * Math.cos(angleRad);
            const y2 = center + (outerRadius + tickLength) * Math.sin(angleRad);
            
            return (
              <line
                key={tickAngle}
                x1={x1}
                y1={y1}
                x2={x2}
                y2={y2}
                stroke="#3b82f6"
                strokeWidth="2"
                strokeLinecap="round"
                opacity="0.6"
              />
            );
          })}
          
          {/* Horizontal line (rotates based on angle) - extends to edge */}
          <g className="leveling-line-rotator" transform={`rotate(${rotation} ${center} ${center})`}>
            <line
              x1={center - outerRadius}
              y1={center}
              x2={center + outerRadius}
              y2={center}
              stroke="#3b82f6"
              strokeWidth="2"
              strokeLinecap="round"
              opacity="1"
            />
          </g>
          
          {/* Center dot */}
          <circle
            cx={center}
            cy={center}
            r="2.5"
            fill="#3b82f6"
          />
        </svg>
      </div>
      <div className="leveling-gauge-value">{displayValue}°</div>
    </div>
  );
};
