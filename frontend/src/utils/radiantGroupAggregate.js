/**
 * Radiant (floor heating) group card: aggregate + master OFF (circles 0–3 in modal).
 */

export const RADIANT_CIRCLE_INDICES = [0, 1, 2, 3];

/**
 * Matches FloorHeatingCard "on" (heating active): TEMP_CONTROL + relay ON.
 * @param {{ mode?: string, relay?: string }|undefined} circle
 */
export function isCircleHeatOnForGroup(circle) {
  const mode = circle?.mode || "OFF";
  const relay = circle?.relay || "OFF";
  return mode === "TEMP_CONTROL" && relay === "ON";
}

/**
 * @param {Record<string, { mode?: string, relay?: string }>} circles
 * @returns {{ anyActive: boolean }}
 */
export function getRadiantGroupAggregate(circles) {
  for (const index of RADIANT_CIRCLE_INDICES) {
    if (isCircleHeatOnForGroup(circles[index])) {
      return { anyActive: true };
    }
  }
  return { anyActive: false };
}

/**
 * Master OFF: same as clicking each zone off when not already OFF.
 * @param {Record<string, { mode?: string }>} circles
 * @returns {{ indicesToOff: number[] }}
 */
export function getRadiantMasterOffPlan(circles) {
  const indicesToOff = [];
  for (const index of RADIANT_CIRCLE_INDICES) {
    const mode = circles[index]?.mode || "OFF";
    if (mode !== "OFF") {
      indicesToOff.push(index);
    }
  }
  return { indicesToOff };
}
