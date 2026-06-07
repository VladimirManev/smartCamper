/**
 * Toilet urine tank detail inside CardModal — vertical fill + level (no temperature).
 */

import { WaterTankModalIcon } from "./WaterTankModalIcon";

/**
 * @param {Object} props
 * @param {number|null} props.level - 0, 50, or 100
 * @param {boolean} props.disabled - module offline
 */
export function ToiletUrineModalContent({ level, disabled = false }) {
  const pct =
    disabled || level === null || level === undefined
      ? 0
      : Math.min(100, Math.max(0, Number(level)));

  return (
    <div className="gray-water-modal toilet-urine-modal">
      <div className="gray-water-modal-tank-wrap">
        <div className="gray-water-modal-tank" aria-hidden="true">
          <div className="gray-water-modal-overlay">
            <WaterTankModalIcon variant="toilet" />
          </div>
          <div className="gray-water-modal-fill" style={{ height: `${pct}%` }} />
        </div>
      </div>
      <div className="gray-water-modal-stats">
        <div className="gray-water-modal-stat">
          <span className="gray-water-modal-stat-label">Level</span>
          <span className="gray-water-modal-stat-value">
            {disabled || level === null || level === undefined
              ? "—"
              : `${Math.round(level)}%`}
          </span>
        </div>
      </div>
    </div>
  );
}
