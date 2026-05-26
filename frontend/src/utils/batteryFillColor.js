/**
 * Battery fill color by state of charge (%).
 * ≤20 red, 21–50 yellow, >50 green.
 */
export function getBatteryFillColor(percent) {
  if (percent === null || percent === undefined || Number.isNaN(Number(percent))) {
    return null;
  }
  const p = Math.min(100, Math.max(0, Number(percent)));
  if (p <= 20) return "#ef4444";
  if (p <= 50) return "#eab308";
  return "#22c55e";
}
