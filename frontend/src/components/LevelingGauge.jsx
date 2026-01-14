/**
 * LevelingGauge Component
 * Displays a circular gauge with horizontal line that rotates based on angle
 * Similar to clock design on main page
 */

import { useState, useEffect } from "react";
import { getThemeColor } from "../utils/getThemeColor";
import IMG_3605 from "../assets/IMG_3605.png";
import IMG_3606 from "../assets/IMG_3606.png";

/**
 * LevelingGauge component
 * @param {Object} props - Component props
 * @param {string} props.label - Label text (e.g., "Pitch" or "Roll")
 * @param {number|null} props.angle - Angle value in degrees (null if no data)
 * @param {string} props.axis - Axis name ("X" for pitch, "Y" for roll)
 */
export const LevelingGauge = ({ label, angle, axis }) => {
  // Get theme colors
  const [accentBlue, setAccentBlue] = useState("#3b82f6");
  const [accentBlueDark, setAccentBlueDark] = useState("#2563eb");
  
  useEffect(() => {
    const updateColors = () => {
      setAccentBlue(getThemeColor("--color-accent-blue"));
      setAccentBlueDark(getThemeColor("--color-accent-blue-dark"));
    };
    
    // Update on mount
    updateColors();
    
    // Update periodically to catch theme changes
    const interval = setInterval(updateColors, 100);
    
    return () => clearInterval(interval);
  }, []);

  // Select image based on axis (swapped due to sensor mounting)
  const imageSrc = axis === "X" ? IMG_3606 : IMG_3605;
  
  // Clamp angle to realistic range for display (-15° to +15°)
  // Real-world scenarios:
  // - Ideal parking: 0-0.5°
  // - Acceptable: up to 1-2°
  // - Moderate slope: 5-8° (10-15% grade)
  // - Steep slope: 8-12° (15-20% grade)
  // - Extreme case: up to 15° (25% grade - very steep hill)
  const clampedAngle = angle !== null && angle !== undefined 
    ? Math.max(-15, Math.min(15, angle))
    : 0;
  
  // Dynamic rotation multiplier: decreases as angle increases
  // Base multiplier for small angles (more sensitive for better visibility at low angles)
  const baseMultiplier = 20;
  // Maximum realistic angle for calculation (15° - extreme steep hill case)
  const maxRealisticAngle = 15;
  // Absolute angle for multiplier calculation
  const absAngle = Math.abs(clampedAngle);
  
  // Calculate dynamic multiplier: decreases more aggressively as angle increases
  // At 0°: multiplier = 20 (very sensitive for small angles - most common case)
  // At 5°: multiplier = 8 (much less sensitive for moderate slopes)
  // At 10°: multiplier = 4 (minimal sensitivity for steep slopes)
  // At 15°: multiplier = 0 (no rotation for extreme cases)
  // Uses piecewise linear function for aggressive reduction:
  // - Linear from 0° to 5°: multiplier = 20 - 2.4*angle
  // - Linear from 5° to 15°: multiplier = 8 - 0.8*(angle-5) = 12 - 0.8*angle
  let dynamicMultiplier;
  if (absAngle <= 5) {
    dynamicMultiplier = baseMultiplier - 2.4 * absAngle;
  } else {
    dynamicMultiplier = 12 - 0.8 * absAngle;
  }
  // Ensure multiplier doesn't go below 0
  dynamicMultiplier = Math.max(0, dynamicMultiplier);
  
  // Convert angle to rotation (negative because positive angle should rotate clockwise)
  // Use dynamic multiplier for more natural rotation
  const rotation = -clampedAngle * dynamicMultiplier;
  
  // Display value in degrees and percentage
  // Convert degrees to percentage: percentage = tan(degrees) * 100
  const degreesValue = angle !== null && angle !== undefined ? angle.toFixed(1) : null;
  const percentageValue = angle !== null && angle !== undefined 
    ? (Math.tan(angle * Math.PI / 180) * 100).toFixed(1)
    : null;
  
  // SVG viewBox dimensions
  const viewBoxSize = 200;
  const center = viewBoxSize / 2;
  const outerRadius = 85;  // Outer circle radius
  // Different sizes for left (X/Pitch) and right (Y/Roll) images
  // Swapped visual styles: X now has Y's style, Y now has X's style
  const innerRadiusX = 84;   // Larger radius for left image (X/Pitch) - swapped from Y
  const innerRadiusY = 55;   // Smaller radius for right image (Y/Roll) - swapped from X
  const tickLength = 8;     // Length of tick marks extending outward
  const lineGap = 60;       // Gap in center of line (very small segments at edges)
  const segmentLength = 15;  // Length of line segments at edges
  
  // Set image dimensions based on axis (swapped styles)
  // Left (X): now has larger circle with narrower/taller image (was Y's style)
  // Right (Y): now has smaller circle (was X's style)
  const imageWidth = axis === "X" ? innerRadiusX * 1.6 : innerRadiusY * 2;  // Swapped
  const imageHeight = axis === "X" ? innerRadiusX * 2.2 : innerRadiusY * 2; // Swapped
  const imageX = axis === "X" ? center - imageWidth / 2 : center - innerRadiusY;
  const imageY = axis === "X" ? center - imageHeight / 2 : center - innerRadiusY;
  
  // Generate tick marks every 10 degrees around full circle (0° to 350°)
  const tickMarks = [];
  for (let angle = 0; angle < 360; angle += 10) {
    tickMarks.push(angle);
  }
  
  return (
    <div className={`leveling-gauge-container leveling-gauge-${axis.toLowerCase()}`}>
      <div className="leveling-gauge">
        <svg className="leveling-gauge-svg" viewBox={`0 0 ${viewBoxSize} ${viewBoxSize}`}>
          <defs>
            <linearGradient id={`leveling-gradient-${axis}`} x1="0%" y1="0%" x2="0%" y2="100%">
              <stop offset="0%" stopColor={accentBlue} />
              <stop offset="100%" stopColor={accentBlueDark} />
            </linearGradient>
            <clipPath id={`leveling-clip-${axis}`}>
              <circle cx={center} cy={center} r={axis === "X" ? innerRadiusX : innerRadiusY} />
            </clipPath>
          </defs>
          
          {/* Outer circle (background) */}
          <circle
            cx={center}
            cy={center}
            r={outerRadius}
            fill="none"
            stroke={getThemeColor("--color-accent-blue-20")}
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
                stroke={accentBlue}
                strokeWidth="2"
                strokeLinecap="round"
                opacity="0.6"
              />
            );
          })}
          
          {/* Rotating group: image and line segments */}
          <g className="leveling-line-rotator" transform={`rotate(${rotation} ${center} ${center})`}>
            {/* Image inside circle - rotates with line */}
            <image
              href={imageSrc}
              x={imageX}
              y={imageY}
              width={imageWidth}
              height={imageHeight}
              clipPath={`url(#leveling-clip-${axis})`}
              opacity="1"
            />
            
            {/* Horizontal line segments (broken in center) - very small segments at edges */}
            {/* Left segment */}
            <line
              x1={center - outerRadius}
              y1={center}
              x2={center - outerRadius + segmentLength}
              y2={center}
              stroke={accentBlue}
              strokeWidth="2"
              strokeLinecap="round"
              opacity="1"
            />
            {/* Right segment */}
            <line
              x1={center + outerRadius - segmentLength}
              y1={center}
              x2={center + outerRadius}
              y2={center}
              stroke={accentBlue}
              strokeWidth="2"
              strokeLinecap="round"
              opacity="1"
            />
          </g>
        </svg>
      </div>
      <div className="leveling-gauge-value">
        {degreesValue !== null ? (
          <>
            <span className="leveling-gauge-degrees">{degreesValue}°</span>
            <span className="leveling-gauge-percentage">({percentageValue}%)</span>
          </>
        ) : (
          "--"
        )}
      </div>
    </div>
  );
};
