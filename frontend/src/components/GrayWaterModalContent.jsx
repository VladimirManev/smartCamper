/**
 * Gray water detail inside CardModal: vertical tank fill + large % and temperature.
 */

import { useEffect, useState } from "react";
import { getThemeColor } from "../utils/getThemeColor";

/**
 * @param {Object} props
 * @param {number|null} props.level - 0–100
 * @param {number|null} props.temperature - °C
 * @param {boolean} props.disabled - module offline
 */
export function GrayWaterModalContent({ level, temperature, disabled = false }) {
  const [fillTop, setFillTop] = useState("#3b82f6");
  const [fillBottom, setFillBottom] = useState("#2563eb");

  useEffect(() => {
    const sync = () => {
      setFillTop(getThemeColor("--color-accent-blue"));
      setFillBottom(getThemeColor("--color-accent-blue-dark"));
    };
    sync();
    const id = setInterval(sync, 2000);
    return () => clearInterval(id);
  }, []);

  const pct =
    disabled || level === null || level === undefined
      ? 0
      : Math.min(100, Math.max(0, Number(level)));

  return (
    <div className="gray-water-modal">
      <div className="gray-water-modal-tank-wrap" aria-hidden="true">
        <div className="gray-water-modal-tank">
          <div
            className="gray-water-modal-fill"
            style={{
              height: `${pct}%`,
              background: `linear-gradient(to top, ${fillBottom} 0%, ${fillTop} 100%)`,
            }}
          />
        </div>
      </div>
      <div className="gray-water-modal-stats">
        <div className="gray-water-modal-stat">
          <span className="gray-water-modal-stat-label">Level</span>
          <span className="gray-water-modal-stat-value">
            {disabled || level === null || level === undefined ? "—" : `${Math.round(level)}%`}
          </span>
        </div>
        <div className="gray-water-modal-stat">
          <span className="gray-water-modal-stat-label">Temp</span>
          <span className="gray-water-modal-stat-value">
            {disabled || temperature === null || temperature === undefined
              ? "—"
              : `${Number(temperature).toFixed(1)}°C`}
          </span>
        </div>
      </div>
    </div>
  );
}
