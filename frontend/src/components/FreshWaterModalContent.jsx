/**
 * Fresh water detail inside CardModal — vertical tank fill + level (no temperature).
 */

import { WaterTankModalIcon } from "./WaterTankModalIcon";

/**
 * @param {Object} props
 * @param {number|null} props.level - 0–100
 * @param {boolean} props.disabled - module offline
 */
export function FreshWaterModalContent({ level, disabled = false }) {
  const pct =
    disabled || level === null || level === undefined
      ? 0
      : Math.min(100, Math.max(0, Number(level)));

  return (
    <div className="gray-water-modal fresh-water-modal">
      <div className="gray-water-modal-tank-wrap">
        <div className="gray-water-modal-tank" aria-hidden="true">
          <div className="gray-water-modal-overlay">
            <WaterTankModalIcon variant="fresh" />
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
