/**
 * Tablet display backlight via module-5 relay 6.
 * Long-press clock to turn off; full-screen tap to wake; optional auto-off idle timer.
 */

import { useCallback, useEffect, useRef, useState } from "react";
import { APPLIANCE_INDEX } from "../constants/appliances";
import { getDisplayAutoOffSeconds } from "../constants/displaySettings";

const WAKE_DEBOUNCE_MS = 400;
const TURN_OFF_WAKE_SUPPRESS_MS = 700;

/**
 * @param {Object} params
 * @param {Object} params.appliances - Appliance relay states from useApplianceController
 * @param {Function} params.sendApplianceCommand
 * @param {boolean} params.isTabletLandscape
 * @param {boolean} params.isModuleOnline - module-5 online
 * @param {string} params.displayAutoOffOption - localStorage value (seconds string or "never")
 * @param {Function} [params.onTurnOff] - Called when backlight is turned off (e.g. reset tablet panel)
 */
export function useDisplayBacklight({
  appliances,
  sendApplianceCommand,
  isTabletLandscape,
  isModuleOnline,
  displayAutoOffOption,
  onTurnOff,
}) {
  const lastWakeAtRef = useRef(0);
  const suppressWakeUntilRef = useRef(0);
  const autoOffTimerRef = useRef(null);
  const [optimisticBacklightOn, setOptimisticBacklightOn] = useState(null);

  const relayBacklightOn =
    appliances[APPLIANCE_INDEX.displayBacklight]?.state === "ON";
  const isBacklightOn =
    optimisticBacklightOn !== null ? optimisticBacklightOn : relayBacklightOn;
  const isFeatureEnabled = isTabletLandscape && isModuleOnline;
  const isAsleep = isFeatureEnabled && !isBacklightOn;

  useEffect(() => {
    if (optimisticBacklightOn === null) {
      return undefined;
    }

    if (relayBacklightOn === optimisticBacklightOn) {
      setOptimisticBacklightOn(null);
    }

    return undefined;
  }, [optimisticBacklightOn, relayBacklightOn]);

  const sendBacklightCommand = useCallback(
    (action) => {
      if (!isFeatureEnabled) {
        return;
      }
      sendApplianceCommand({
        type: "relay",
        index: APPLIANCE_INDEX.displayBacklight,
        action,
      });
    },
    [isFeatureEnabled, sendApplianceCommand]
  );

  const turnOn = useCallback(() => {
    const now = Date.now();
    if (now < suppressWakeUntilRef.current) {
      return;
    }
    if (now - lastWakeAtRef.current < WAKE_DEBOUNCE_MS) {
      return;
    }
    lastWakeAtRef.current = now;
    setOptimisticBacklightOn(true);
    sendBacklightCommand("on");
  }, [sendBacklightCommand]);

  const turnOff = useCallback(() => {
    if (!isFeatureEnabled || !isBacklightOn) {
      return;
    }
    suppressWakeUntilRef.current = Date.now() + TURN_OFF_WAKE_SUPPRESS_MS;
    setOptimisticBacklightOn(false);
    sendBacklightCommand("off");
    onTurnOff?.();
  }, [isFeatureEnabled, isBacklightOn, sendBacklightCommand, onTurnOff]);

  const turnOffRef = useRef(turnOff);
  turnOffRef.current = turnOff;

  const clearAutoOffTimer = useCallback(() => {
    if (autoOffTimerRef.current) {
      clearTimeout(autoOffTimerRef.current);
      autoOffTimerRef.current = null;
    }
  }, []);

  const startAutoOffTimer = useCallback(() => {
    clearAutoOffTimer();

    const idleSeconds = getDisplayAutoOffSeconds(displayAutoOffOption);
    if (
      !isTabletLandscape ||
      !isModuleOnline ||
      !isBacklightOn ||
      idleSeconds === null
    ) {
      return;
    }

    autoOffTimerRef.current = setTimeout(() => {
      turnOffRef.current();
    }, idleSeconds * 1000);
  }, [
    clearAutoOffTimer,
    displayAutoOffOption,
    isBacklightOn,
    isModuleOnline,
    isTabletLandscape,
  ]);

  useEffect(() => {
    if (!isTabletLandscape || !isModuleOnline || !isBacklightOn) {
      clearAutoOffTimer();
      return undefined;
    }

    const idleSeconds = getDisplayAutoOffSeconds(displayAutoOffOption);
    if (idleSeconds === null) {
      clearAutoOffTimer();
      return undefined;
    }

    startAutoOffTimer();

    const resetTimer = () => {
      startAutoOffTimer();
    };

    const events = ["pointerdown", "touchstart", "keydown"];
    for (const eventName of events) {
      document.addEventListener(eventName, resetTimer, { passive: true });
    }

    return () => {
      clearAutoOffTimer();
      for (const eventName of events) {
        document.removeEventListener(eventName, resetTimer);
      }
    };
  }, [
    clearAutoOffTimer,
    displayAutoOffOption,
    isBacklightOn,
    isModuleOnline,
    isTabletLandscape,
    startAutoOffTimer,
  ]);

  return {
    isBacklightOn,
    isAsleep,
    isFeatureEnabled,
    turnOn,
    turnOff,
  };
}
