/**
 * Aggregate state for the main "Light" group card (lighting modal strips + Ambient relay).
 * Bathroom strip (3): AUTO counts as off for the group; only non-AUTO ON counts.
 */

/** Strip indices included in the lighting group modal */
export const LIGHTING_GROUP_STRIP_INDICES = [0, 1, 3, 4];

/** Ambient in the same modal — module-2 relay index */
export const AMBIENT_RELAY_INDEX = 0;

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
 * @param {Record<string, { state?: string }>} relays
 * @returns {{ anyActive: boolean }}
 */
export function getLightingGroupAggregate(ledStrips, relays) {
  for (const index of LIGHTING_GROUP_STRIP_INDICES) {
    const strip = ledStrips[index];
    if (isStripActiveForLightingGroup(index, strip)) {
      return { anyActive: true };
    }
  }
  const ambient = relays?.[AMBIENT_RELAY_INDEX];
  if (ambient?.state === "ON") {
    return { anyActive: true };
  }
  return { anyActive: false };
}
