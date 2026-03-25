/**
 * Aggregate state for the main "Light" group card (strips shown in lighting modal).
 * Bathroom strip (3): AUTO counts as off for the group; only non-AUTO ON counts.
 */

/** Strip indices included in the lighting group modal */
export const LIGHTING_GROUP_STRIP_INDICES = [0, 1, 3, 4];

const BATHROOM_STRIP_INDEX = 3;

/**
 * @param {number} index - Strip index
 * @param {{ state?: string, brightness?: number, mode?: string }|undefined} strip
 * @returns {boolean}
 */
export function isStripActiveForLightingGroup(index, strip) {
  if (!strip || strip.state !== "ON") return false;
  if (index === BATHROOM_STRIP_INDEX && strip.mode === "AUTO") return false;
  return true;
}

/**
 * @param {Record<string, { state?: string, brightness?: number, mode?: string }>} ledStrips
 * @returns {{ anyActive: boolean, maxBrightness: number }}
 */
export function getLightingGroupAggregate(ledStrips) {
  let anyActive = false;
  let maxBrightness = 0;
  for (const index of LIGHTING_GROUP_STRIP_INDICES) {
    const strip = ledStrips[index];
    if (isStripActiveForLightingGroup(index, strip)) {
      anyActive = true;
      const b = strip.brightness ?? 0;
      if (b > maxBrightness) maxBrightness = b;
    }
  }
  return { anyActive, maxBrightness };
}
