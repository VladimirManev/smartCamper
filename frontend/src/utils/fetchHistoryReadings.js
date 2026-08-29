/**
 * Fetch history readings from the Pi backend.
 * @param {Object} options
 * @param {string} options.metric
 * @param {number} [options.hours=24]
 * @param {AbortSignal} [options.signal]
 * @returns {Promise<{ metric: string, unit: string|null, hours: number, points: Array<{ ts: number, value: number }> }>}
 */
import { getBackendHttpBase } from "./getBackendHttpBase";

export async function fetchHistoryReadings({
  metric,
  hours = 24,
  signal,
} = {}) {
  if (!metric) {
    throw new Error("metric is required");
  }

  const base = getBackendHttpBase();
  const url = `${base}/api/history/readings?metric=${encodeURIComponent(
    metric
  )}&hours=${encodeURIComponent(String(hours))}`;

  const response = await fetch(url, { signal });
  if (!response.ok) {
    let detail = response.statusText;
    try {
      const body = await response.json();
      if (body?.error) detail = body.error;
    } catch (_) {
      // ignore
    }
    throw new Error(detail || `HTTP ${response.status}`);
  }

  return response.json();
}
