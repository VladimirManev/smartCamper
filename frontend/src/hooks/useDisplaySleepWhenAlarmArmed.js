/**
 * Turn display off 10s after zone1 reaches armed (normal or cat).
 * Skips / cancels if phase leaves armed.
 */

import { useEffect, useRef } from "react";

const SLEEP_AFTER_ARMED_MS = 10_000;

/**
 * @param {Object} params
 * @param {string} params.zone1Phase
 * @param {boolean} params.isFeatureEnabled
 * @param {boolean} params.isModuleOnline
 * @param {Function} params.turnOff
 */
export function useDisplaySleepWhenAlarmArmed({
  zone1Phase,
  isFeatureEnabled,
  isModuleOnline,
  turnOff,
}) {
  const turnOffRef = useRef(turnOff);
  turnOffRef.current = turnOff;

  useEffect(() => {
    if (!isModuleOnline || !isFeatureEnabled || zone1Phase !== "armed") {
      return undefined;
    }

    const id = setTimeout(() => {
      turnOffRef.current();
    }, SLEEP_AFTER_ARMED_MS);

    return () => clearTimeout(id);
  }, [zone1Phase, isFeatureEnabled, isModuleOnline]);
}
