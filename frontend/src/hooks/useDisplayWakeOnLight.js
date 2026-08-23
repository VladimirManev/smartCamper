/**
 * Wake tablet display backlight when any light zone turns on while display is asleep.
 */

import { useEffect, useRef } from "react";
import { isAnyLightOnForDisplayWake } from "../utils/lightingGroupAggregate";

/**
 * @param {Object} params
 * @param {Record<string, object>} params.ledStrips
 * @param {Record<string, object>} params.relays
 * @param {boolean} params.isAsleep
 * @param {boolean} params.isFeatureEnabled
 * @param {boolean} params.isModuleOnline - module-2 online
 * @param {Function} params.turnOn
 */
export function useDisplayWakeOnLight({
  ledStrips,
  relays,
  isAsleep,
  isFeatureEnabled,
  isModuleOnline,
  turnOn,
}) {
  const prevLightOnRef = useRef(false);
  const prevIsAsleepRef = useRef(false);
  const skipInitialRef = useRef(true);
  const turnOnRef = useRef(turnOn);
  turnOnRef.current = turnOn;

  useEffect(() => {
    if (!isFeatureEnabled || !isModuleOnline) {
      return;
    }

    const lightOn = isAnyLightOnForDisplayWake(ledStrips, relays);

    if (skipInitialRef.current) {
      skipInitialRef.current = false;
      prevLightOnRef.current = lightOn;
      prevIsAsleepRef.current = isAsleep;
      return;
    }

    const justEnteredSleep = isAsleep && !prevIsAsleepRef.current;
    if (justEnteredSleep) {
      prevLightOnRef.current = lightOn;
      prevIsAsleepRef.current = isAsleep;
      return;
    }

    if (isAsleep && prevIsAsleepRef.current && !prevLightOnRef.current && lightOn) {
      turnOnRef.current();
    }

    prevLightOnRef.current = lightOn;
    prevIsAsleepRef.current = isAsleep;
  }, [ledStrips, relays, isAsleep, isFeatureEnabled, isModuleOnline]);
}
