/**
 * Extract key Victron metrics and decide which readings to write.
 * Thresholds: SOC ≥ 1 pp, V ≥ 0.1 V, A ≥ 0.5 A, solar ≥ 10 W,
 * Orion output A ≥ 0.5 A, or ≥ 60 s since last sample for that metric.
 */

const MAX_INTERVAL_MS = 60_000;

const THRESHOLDS = {
  soc: 1,
  voltage: 0.1,
  current: 0.5,
  solar_w: 10,
  orion_output_a: 0.5,
};

const UNITS = {
  soc: "%",
  voltage: "V",
  current: "A",
  solar_w: "W",
  orion_output_a: "A",
};

/**
 * @param {Object|null|undefined} data - Victron status JSON (module-6)
 * @returns {{ soc: number|null, voltage: number|null, current: number|null, solar_w: number|null, orion_output_a: number|null }}
 */
function extractVictronMetrics(data) {
  if (!data || typeof data !== "object") {
    return {
      soc: null,
      voltage: null,
      current: null,
      solar_w: null,
      orion_output_a: null,
    };
  }

  const shunt = data.smartshunt;
  const mppt1 = data.mppt1;
  const mppt2 = data.mppt2;
  const orion = data.orion;

  let solar_w = null;
  const p1 = mppt1 && typeof mppt1.pvPower === "number" ? mppt1.pvPower : null;
  const p2 = mppt2 && typeof mppt2.pvPower === "number" ? mppt2.pvPower : null;
  if (p1 != null || p2 != null) {
    solar_w = (p1 || 0) + (p2 || 0);
  }

  return {
    soc: shunt && typeof shunt.soc === "number" ? shunt.soc : null,
    voltage: shunt && typeof shunt.voltage === "number" ? shunt.voltage : null,
    current: shunt && typeof shunt.current === "number" ? shunt.current : null,
    solar_w,
    orion_output_a:
      orion && typeof orion.outputCurrent === "number"
        ? orion.outputCurrent
        : null,
  };
}

/**
 * @param {string} metric
 * @param {number} value
 * @param {{ value: number, ts: number }|undefined} last
 * @param {number} now
 * @returns {boolean}
 */
function shouldWriteMetric(metric, value, last, now) {
  if (value == null || Number.isNaN(value)) {
    return false;
  }
  if (!last) {
    return true;
  }
  if (now - last.ts >= MAX_INTERVAL_MS) {
    return true;
  }
  const threshold = THRESHOLDS[metric];
  if (threshold == null) {
    return true;
  }
  return Math.abs(value - last.value) >= threshold;
}

/**
 * Decide which Victron metrics to persist now.
 * @param {Object} data - Victron status JSON
 * @param {Map<string, { value: number, ts: number }>} lastByMetric
 * @param {number} [now]
 * @returns {Array<{ metric: string, value: number, unit: string, source: string }>}
 */
function selectVictronReadings(data, lastByMetric, now = Date.now()) {
  const metrics = extractVictronMetrics(data);
  const out = [];

  for (const [metric, value] of Object.entries(metrics)) {
    if (!shouldWriteMetric(metric, value, lastByMetric.get(metric), now)) {
      continue;
    }
    out.push({
      metric,
      value,
      unit: UNITS[metric] || null,
      source: "module-6",
    });
  }

  return out;
}

module.exports = {
  extractVictronMetrics,
  selectVictronReadings,
  THRESHOLDS,
  MAX_INTERVAL_MS,
};
