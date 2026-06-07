/** Card circle turns red when level crosses these thresholds. */

export const FRESH_WATER_CRITICAL_MAX = 15;
export const GRAY_WATER_CRITICAL_MIN = 75;
export const TOILET_URINE_CRITICAL_MIN = 100;

export function isFreshWaterCritical(level) {
  return level !== null && level !== undefined && level <= FRESH_WATER_CRITICAL_MAX;
}

export function isGrayWaterCritical(level) {
  return level !== null && level !== undefined && level >= GRAY_WATER_CRITICAL_MIN;
}

export function isToiletUrineCritical(level) {
  return level !== null && level !== undefined && level >= TOILET_URINE_CRITICAL_MIN;
}
