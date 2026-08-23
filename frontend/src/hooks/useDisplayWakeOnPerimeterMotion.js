/**
 * Wake tablet display and open perimeter modal on new motion while zone 2 is armed.
 */

import { useEffect, useRef } from "react";

/**
 * @param {Object} params
 * @param {boolean} params.zone2Armed
 * @param {Record<string, number|null>} params.perimeterLastMotion
 * @param {boolean} params.isAsleep
 * @param {boolean} params.isFeatureEnabled
 * @param {boolean} params.isModuleOnline - module-8 online
 * @param {Function} params.turnOn
 * @param {Function} params.openPerimeterModal
 */
export function useDisplayWakeOnPerimeterMotion({
  zone2Armed,
  perimeterLastMotion,
  isAsleep,
  isFeatureEnabled,
  isModuleOnline,
  turnOn,
  openPerimeterModal,
}) {
  const prevMotionRef = useRef({});
  const skipInitialRef = useRef(true);
  const turnOnRef = useRef(turnOn);
  const openPerimeterModalRef = useRef(openPerimeterModal);

  turnOnRef.current = turnOn;
  openPerimeterModalRef.current = openPerimeterModal;

  useEffect(() => {
    if (!zone2Armed || !isModuleOnline) {
      prevMotionRef.current = {};
      skipInitialRef.current = true;
      return;
    }

    if (skipInitialRef.current) {
      skipInitialRef.current = false;
      prevMotionRef.current = { ...perimeterLastMotion };
      return;
    }

    const prev = prevMotionRef.current;
    const hasNewMotion = Object.entries(perimeterLastMotion).some(
      ([sensor, ts]) => ts != null && ts !== prev[sensor]
    );

    if (hasNewMotion && isFeatureEnabled && isAsleep) {
      turnOnRef.current();
      openPerimeterModalRef.current();
    }

    prevMotionRef.current = { ...perimeterLastMotion };
  }, [
    zone2Armed,
    perimeterLastMotion,
    isAsleep,
    isFeatureEnabled,
    isModuleOnline,
  ]);
}
