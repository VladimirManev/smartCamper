const SHIELD_EVENTS = [
  "click",
  "mousedown",
  "mouseup",
  "pointerdown",
  "pointerup",
  "touchstart",
  "touchend",
];

/**
 * Block ghost clicks after the wake overlay is removed (capture phase).
 * @param {number} durationMs
 * @returns {() => void} cleanup
 */
export function installClickShield(durationMs = 500) {
  const block = (event) => {
    event.preventDefault();
    event.stopPropagation();
    event.stopImmediatePropagation();
  };

  for (const eventName of SHIELD_EVENTS) {
    document.addEventListener(eventName, block, true);
  }

  const timeoutId = setTimeout(() => {
    for (const eventName of SHIELD_EVENTS) {
      document.removeEventListener(eventName, block, true);
    }
  }, durationMs);

  return () => {
    clearTimeout(timeoutId);
    for (const eventName of SHIELD_EVENTS) {
      document.removeEventListener(eventName, block, true);
    }
  };
}
