/**
 * Wake display + open Alarm modal on zone1 entry_delay / alarm.
 * Also open Alarm modal when exit_delay starts (e.g. physical button).
 */

import { useEffect, useRef } from "react";

const THREAT_PHASES = new Set(["entry_delay", "alarm"]);
const EXIT_PHASES = new Set(["exit_delay"]);

/**
 * @param {Object} params
 * @param {string} params.zone1Phase
 * @param {boolean} params.isAsleep
 * @param {boolean} params.isFeatureEnabled
 * @param {boolean} params.isModuleOnline
 * @param {Function} params.turnOn
 * @param {Function} params.openAlarmModal
 */
export function useDisplayWakeOnAlarm({
  zone1Phase,
  isAsleep,
  isFeatureEnabled,
  isModuleOnline,
  turnOn,
  openAlarmModal,
}) {
  const prevPhaseRef = useRef(zone1Phase);
  const turnOnRef = useRef(turnOn);
  const openAlarmModalRef = useRef(openAlarmModal);

  turnOnRef.current = turnOn;
  openAlarmModalRef.current = openAlarmModal;

  useEffect(() => {
    if (!isModuleOnline) {
      prevPhaseRef.current = zone1Phase;
      return;
    }

    const prev = prevPhaseRef.current;
    prevPhaseRef.current = zone1Phase;

    if (prev === zone1Phase) return;

    if (EXIT_PHASES.has(zone1Phase) && !EXIT_PHASES.has(prev)) {
      openAlarmModalRef.current();
      if (isFeatureEnabled && isAsleep) {
        turnOnRef.current();
      }
      return;
    }

    if (THREAT_PHASES.has(zone1Phase) && !THREAT_PHASES.has(prev)) {
      openAlarmModalRef.current();
      if (isFeatureEnabled && isAsleep) {
        turnOnRef.current();
      }
    }
  }, [zone1Phase, isAsleep, isFeatureEnabled, isModuleOnline]);
}
