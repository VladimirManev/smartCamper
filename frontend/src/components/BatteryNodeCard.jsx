/**
 * Compact peripheral node on the battery energy diagram.
 */

import { formatBatteryNodeMetrics } from "../utils/formatBatteryMetrics";

/**
 * @param {Object} props
 * @param {string} props.label
 * @param {string} props.icon - Font Awesome class without "fa-solid" prefix (e.g. "fa-sun")
 * @param {Object|null} props.data - metrics from backend / mock
 * @param {'voltageCurrent' | 'powerVoltage'} props.metricKind
 * @param {boolean} props.disabled
 */
export function BatteryNodeCard({
  label,
  icon,
  data,
  metricKind,
  disabled = false,
}) {
  const metrics = disabled ? "—" : formatBatteryNodeMetrics(data, metricKind);

  return (
    <div className={`battery-node-card ${disabled ? "battery-node-card--disabled" : ""}`}>
      <div className="battery-node-card__icon" aria-hidden="true">
        <i className={`fa-solid ${icon}`} />
      </div>
      <div className="battery-node-card__label">{label}</div>
      <div className="battery-node-card__metrics">{metrics}</div>
    </div>
  );
}
