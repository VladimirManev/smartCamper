/**
 * Gray water detail inside CardModal: vertical tank fill + level and temperature.
 */

import { WaterTankModalIcon } from "./WaterTankModalIcon";
import { ModalStatSlots } from "./ModalStatSlots";

/**
 * @param {Object} props
 * @param {number|null} props.level - 0–100
 * @param {number|null} props.temperature - °C
 * @param {boolean} props.disabled - module offline
 */
export function GrayWaterModalContent({ level, temperature, disabled = false }) {
  const pct =
    disabled || level === null || level === undefined
      ? 0
      : Math.min(100, Math.max(0, Number(level)));

  const levelValue =
    disabled || level === null || level === undefined
      ? "—"
      : `${Math.round(level)}%`;
  const tempValue =
    disabled || temperature === null || temperature === undefined
      ? "—"
      : `${Number(temperature).toFixed(1)}°C`;

  return (
    <div className="gray-water-modal">
      <div className="gray-water-modal-tank-wrap">
        <div className="gray-water-modal-tank" aria-hidden="true">
          <div className="gray-water-modal-overlay">
            <WaterTankModalIcon variant="gray" />
          </div>
          <div className="gray-water-modal-fill" style={{ height: `${pct}%` }} />
        </div>
      </div>
      <ModalStatSlots
        items={[
          { label: "Level", value: levelValue },
          { label: "Temp", value: tempValue },
        ]}
      />
    </div>
  );
}
