/**
 * Radiant (floor heating) group card: aggregate + master OFF (circles 0–3 in modal).
 */

export const RADIANT_CIRCLE_INDICES = [0, 1, 2, 3];

/**
 * Group card "on" if the zone is not fully off (TEMP_CONTROL regardless of relay/temp).
 * @param {{ mode?: string }|undefined} circle
 */
export function isCircleOnForRadiantGroup(circle) {
  return (circle?.mode || "OFF") !== "OFF";
}

/**
 * @param {Record<string, { mode?: string }>} circles
 * @returns {{ anyActive: boolean }}
 */
export function getRadiantGroupAggregate(circles) {
  for (const index of RADIANT_CIRCLE_INDICES) {
    if (isCircleOnForRadiantGroup(circles[index])) {
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
