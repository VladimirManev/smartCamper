/**
 * ToiletUrineTank — same tank visualization as gray water, level only (no temperature).
 */

import { useEffect, useState, useRef } from "react";
import { getThemeColor } from "../utils/getThemeColor";
import { isToiletUrineCritical } from "../utils/waterLevelCritical";
import { Card } from "./Card";
import { WaterTankCriticalBadge } from "./WaterTankCriticalBadge";

/**
 * @param {Object} props
 * @param {number|null} props.level - Level percentage (0, 50, or 100)
 * @param {boolean} props.disabled - module offline
 * @param {Function} [props.onClick]
 * @param {Function} [props.onLongPress]
 */
export const ToiletUrineTank = ({
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
  const isCritical = isToiletUrineCritical(displayLevel);

  const tankContent = (
    <div className="water-tank-content">
      <span
        className="water-tank-card-label"
        style={{ color: textPrimary }}
        aria-hidden="true"
      >
        WC
      </span>
      {readoutText !== null && (
        <div className="water-tank-readout" style={{ color: textPrimary }}>
          {readoutText}
        </div>
      )}
    </div>
  );

  return (
    <div ref={cardRef} className="toilet-urine-card-root water-tank-card-root">
      <WaterTankCriticalBadge show={isCritical} />
      <Card
        name="Toilet"
        icon={null}
        buttonState="off"
        iconState={disabled ? "gray" : "active"}
        disabled={disabled}
        cardClass="water-tank-card"
        onClick={onClick}
        onLongPress={onLongPress}
      >
        {tankContent}
      </Card>
    </div>
  );
};
