/**
 * ECGIndicator Component
 * Displays a heartbeat-like ECG waveform indicator
 * Changes color based on data freshness (blue if data received within 1s, red otherwise)
 */

import { useState, useEffect } from "react";

/**
 * ECGIndicator component
 * @param {Object} props - Component props
 * @param {number|null} props.lastDataTimestamp - Timestamp of last received data
 */
export const ECGIndicator = ({ lastDataTimestamp }) => {
  const [isActive, setIsActive] = useState(false);

  // Check if data is fresh (within 1 second)
  useEffect(() => {
    const checkDataFreshness = () => {
      if (lastDataTimestamp === null) {
        setIsActive(false);
        return;
      }

      const timeSinceLastData = Date.now() - lastDataTimestamp;
      setIsActive(timeSinceLastData < 1000); // Active if data received within 1 second
    };

    checkDataFreshness();
    
    // Check every 100ms for smooth color transitions
    const interval = setInterval(checkDataFreshness, 100);

    return () => clearInterval(interval);
  }, [lastDataTimestamp]);

  // Color based on data freshness
  const color = isActive ? "var(--color-accent-blue)" : "var(--color-accent-red)"; // Blue if active, red if inactive

  // ECG waveform path - represents one heartbeat
  // Path: flat line -> spike up -> down -> small bump -> flat line
  // Made much narrower to fit better between circles
  const ecgPath = "M 0 20 L 8 20 L 10 5 L 12 30 L 14 10 L 16 20 L 30 20";

  return (
    <div className="ecg-indicator">
      <svg 
        className="ecg-svg" 
        viewBox="0 0 30 40" 
        width="30" 
        height="40"
        preserveAspectRatio="xMidYMid meet"
      >
        <path
          d={ecgPath}
          fill="none"
          stroke={color}
          strokeWidth="2.5"
          strokeLinecap="round"
          strokeLinejoin="round"
          className="ecg-path"
        />
      </svg>
    </div>
  );
};
