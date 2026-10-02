/**
 * Fresh water detail inside CardModal — vertical tank fill + level (no temperature).
 */

import { WaterTankModalIcon } from "./WaterTankModalIcon";
import { ModalStatSlots } from "./ModalStatSlots";

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

  const levelValue =
    disabled || level === null || level === undefined
      ? "—"
      : `${Math.round(level)}%`;

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
      <ModalStatSlots items={[{ label: "Level", value: levelValue }]} />
    </div>
  );
}
