/**
 * GrayWaterTank Component
 * Displays gray water level visualization using Card component
 * The button itself acts as the tank and fills from bottom up
 */

import { useEffect, useState, useRef } from "react";
import { getThemeColor } from "../utils/getThemeColor";
import { isGrayWaterCritical } from "../utils/waterLevelCritical";
import { Card } from "./Card";
import { WaterTankCriticalBadge } from "./WaterTankCriticalBadge";

/**
 * GrayWaterTank component
 * @param {Object} props - Component props
 * @param {number|null} props.level - Water level percentage (0-100)
 * @param {number|null} props.temperature - Water temperature in Celsius
 * @param {boolean} props.disabled - Whether the sensor is disabled/offline
 * @param {Function} [props.onClick] - Opens detail modal on tap (same as other main cards)
 * @param {Function} [props.onLongPress] - Optional long-press (e.g. same modal)
 */
export const GrayWaterTank = ({ level, temperature, disabled = false, onClick, onLongPress }) => {
  // When disabled, don't show water (empty tank)
  const displayLevel = disabled ? null : level;
  const cardRef = useRef(null);
  /** When both level and temperature exist, false = show °C, true = show % */
  const [showLevelReadout, setShowLevelReadout] = useState(false);

  // Get theme colors
  const [textPrimary, setTextPrimary] = useState("#f5f5f5");

  useEffect(() => {
    const updateColors = () => {
      setTextPrimary(getThemeColor("--color-text-primary"));
    };
    updateColors();
    const interval = setInterval(updateColors, 2000);
    return () => clearInterval(interval);
  }, []);

  useEffect(() => {
    if (cardRef.current) {
      const button = cardRef.current.querySelector(".neumorphic-button");
      if (button) {
        button.style.setProperty(
          "--water-level",
          displayLevel !== null && displayLevel !== undefined
            ? `${displayLevel}%`
            : "0%"
        );
      }
    }
  }, [displayLevel]);

  const hasLevel =
    !disabled &&
    displayLevel !== null &&
    displayLevel !== undefined;
  const hasTemp =
    !disabled &&
    temperature !== null &&
    temperature !== undefined;

  useEffect(() => {
    if (!hasLevel || !hasTemp) return;
    const id = setInterval(() => {
      setShowLevelReadout((v) => !v);
    }, 2000);
    return () => clearInterval(id);
  }, [hasLevel, hasTemp]);

  let readoutText = null;
  if (hasLevel && hasTemp) {
    readoutText = showLevelReadout
      ? `${Math.round(displayLevel)}%`
      : `${temperature.toFixed(1)}°`;
  } else if (hasLevel) {
    readoutText = `${Math.round(displayLevel)}%`;
  } else if (hasTemp) {
    readoutText = `${temperature.toFixed(1)}°`;
  }

  const isCritical = isGrayWaterCritical(displayLevel);

  // Text content to display in the center of the button
  const waterTankContent = (
    <div className="water-tank-content">
      <i className="fas fa-droplet water-tank-card-icon" aria-hidden="true" />
      {readoutText !== null && (
        <div
          className="water-tank-readout"
          style={{
            color: textPrimary,
          }}
        >
          {readoutText}
        </div>
      )}
    </div>
  );

  return (
    <div ref={cardRef} className="gray-water-card-root water-tank-card-root">
      <WaterTankCriticalBadge show={isCritical} />
      <Card
        name="Gray Water"
        icon={null}
        buttonState="off"
        iconState={disabled ? "gray" : "active"}
        disabled={disabled}
        cardClass="water-tank-card"
        onClick={onClick}
        onLongPress={onLongPress}
      >
        {waterTankContent}
      </Card>
    </div>
  );
};

