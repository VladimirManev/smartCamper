/**
 * GrayWaterTank Component
 * Displays gray water level visualization using Card component
 * The button itself acts as the tank and fills from bottom up
 */

import { useEffect, useState, useRef } from "react";
import { getThemeColor } from "../utils/getThemeColor";
import { Card } from "./Card";

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
  const cardRef = useRef(null);
  
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

  // Update CSS custom properties on the button element
  useEffect(() => {
    if (cardRef.current) {
      const button = cardRef.current.querySelector('.neumorphic-button');
      if (button) {
        button.style.setProperty('--water-level', displayLevel !== null && displayLevel !== undefined ? `${displayLevel}%` : '0%');
        button.style.setProperty('--water-color-top', accentBlue);
        button.style.setProperty('--water-color-bottom', accentBlueDark);
      }
    }
  }, [displayLevel, accentBlue, accentBlueDark]);

  // Text content to display in the center of the button
  const waterTankContent = (
    <div className="water-tank-content">
      {displayLevel !== null && displayLevel !== undefined && !disabled && (
        temperature !== null && temperature !== undefined && (
          <div
            className="water-tank-temperature"
            style={{
              color: textPrimary,
            }}
          >
            {temperature.toFixed(1)}°
          </div>
        )
      )}
    </div>
  );

  return (
    <div ref={cardRef}>
      <Card
        name=""
        icon={null}
        buttonState="off"
        iconState={disabled ? "gray" : "active"}
        disabled={disabled}
        cardClass="water-tank-card"
      >
        {waterTankContent}
      </Card>
    </div>
  );
};

