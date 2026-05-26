/**
 * Format node metrics for battery diagram cards.
 */

/**
 * @param {Object|null|undefined} data
 * @param {'voltageCurrent' | 'powerVoltage'} kind
 * @returns {string}
 */
export function formatBatteryNodeMetrics(data, kind) {
  if (!data || typeof data !== "object") {
    return "—";
  }

  if (kind === "voltageCurrent") {
    const v = data.voltage;
    const a = data.current;
    if (v == null && a == null) return "—";
    const vs = v != null ? `${Number(v).toFixed(1)}V` : "—";
    const as = a != null ? `${Number(a).toFixed(1)}A` : "—";
    return `${vs} • ${as}`;
  }

  const w = data.power;
  const v = data.voltage;
  if (w == null && v == null) return "—";
  const ws = w != null ? `${Math.round(Number(w))}W` : "—";
  const vs = v != null ? `${Number(v).toFixed(1)}V` : "—";
  return `${ws} • ${vs}`;
}
