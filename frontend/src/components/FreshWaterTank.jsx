/**
 * FreshWaterTank — clean water level card (same tank visualization as gray water).
 */

import { useEffect, useState, useRef } from "react";
import { getThemeColor } from "../utils/getThemeColor";
import { isFreshWaterCritical } from "../utils/waterLevelCritical";
import { Card } from "./Card";
import { WaterTankCriticalBadge } from "./WaterTankCriticalBadge";

/**
 * @param {Object} props
 * @param {number|null} props.level - Water level percentage (0-100)
 * @param {boolean} props.disabled - Whether the sensor is disabled/offline
 * @param {Function} [props.onClick]
 * @param {Function} [props.onLongPress]
 */
export const FreshWaterTank = ({
  level,
  disabled = false,
  onClick,
  onLongPress,
}) => {
  const displayLevel = disabled ? null : level;
  const cardRef = useRef(null);

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
    !disabled && displayLevel !== null && displayLevel !== undefined;

  const readoutText = hasLevel ? `${Math.round(displayLevel)}%` : null;
  const isCritical = isFreshWaterCritical(displayLevel);

  const waterTankContent = (
    <div className="water-tank-content">
      <i className="fas fa-faucet water-tank-card-icon" aria-hidden="true" />
      {readoutText !== null && (
        <div className="water-tank-readout" style={{ color: textPrimary }}>
          {readoutText}
        </div>
      )}
    </div>
  );

  return (
    <div ref={cardRef} className="fresh-water-card-root water-tank-card-root">
      <WaterTankCriticalBadge show={isCritical} />
      <Card
        name="Fresh Water"
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
